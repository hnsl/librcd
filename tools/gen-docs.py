#!/usr/bin/env python3
from string import Template
import subprocess
import glob
import time
import sys
import os
import re

current_date = time.strftime("%Y-%m-%d")

def print_warning(text, link):
    filename, line = link.split('/')[-1].split('#L')
    text = "\033[91mwarning:\033[0m " + text + " (" + filename + ":" + line + ")"
    # sys.stderr.write(text + '\n')
    print(text)

def esc(str):
    return str.replace('&', '&amp;').replace('<', '&lt;').replace('"', '&quot;')

def output_doc(output, doc, indent = ''):
    if not doc:
        return
    output.append("<div class=doc>")
    started_example = False
    prev_line = ""
    for line in doc:
        if not line:
            if started_example or prev_line[-1:] == ":":
                started_example = not started_example
                output.append("\n<pre>" if started_example else "</pre>")
            else:
                output.append("<p>")
        else:
            output.append(esc(line))
        prev_line = line
    if started_example:
        output.append("</pre>")
    output.append("</div>")

def output_def(output, before, name, after, link):
    link_html = "<a href={} class=source>Source</a>".format(esc(link))
    output.append("<div class=def id=\"{}\">{}<b>{}</b>{}{}</div>".format(esc(name), esc(before), esc(name), esc(after), link_html))

def output_defbegin(output):
    output.append("<div class=block>")

def output_defend(output):
    output.append("</div>")

def output_header(output, header_name, revision):
    output.append(Template('''\
<!DOCTYPE html>
<html>
<head>
<title>$header_name</title>
<meta charset=utf-8>
<link rel=stylesheet href=doc.css>
</head>
<body>\
<h1>Documentation for $header_name</h1>
<h3>last update: $date, revision: $revision</h3>
''').substitute(header_name=header_name, revision=revision, date=current_date))

def output_footer(output):
    output.append('''\
</body>
</html>\
''')

re_ignore = re.compile('''^__attribute__\(\(.*\)\)$''')
def ignore_line(line):
    return re_ignore.match(line)

def ignore_name(name):
    return name.startswith('_') or name.startswith('rcd_pp')

def ignore_bad_line(line, link):
    print_warning("ignoring |" + line + "|", link)

def output_struct(output, name, doc, link, content):
    # TODO (struct are weird, and their members need parsing)
    output_defbegin(output)
    orig_doc = doc
    data = "typedef struct {\n"
    for (line, doc, link) in content:
        if re.match('''^\s*//''', line):
            continue
        # output_doc(output, doc, "  ")
        for dline in doc:
            data += "    // " + dline + "\n"
        data += line + "\n"
    data += "} ";
    output_def(output, data, name, ";", link)
    output_doc(output, orig_doc)
    output_defend(output)

def output_enum(output, name, doc, link, content):
    # TODO
    output_defbegin(output)
    data = "typedef enum {\n"
    orig_doc = doc
    for (line, doc, link) in content:
        # output_doc(output, doc, "  ")
        for dline in doc:
            data += "    // " + dline + "\n"
        m = re.match('''^\s*(\w+)(?:\s*=\s*(\w+))?,''', line)
        assert m, "invalid enum line: |" + line + "|"
        name = m.group(1)
        value = m.group(2)
        if value:
            data += "    " + name + ' = ' + value + ",\n"
        else:
            data += "    " + name + ",\n"
    data += "} "
    output_def(output, data, name, ";", link)
    output_doc(output, orig_doc)
    output_defend(output)

def output_typedef(output, name, before, after, doc, link):
    output_defbegin(output)
    output_def(output, "typedef " + before, name, after + ';', link)
    output_doc(output, doc)
    output_defend(output)

def output_macro(output, name, args, single_line_expansion, doc, link):
    arg_text = ''
    if args is not None:
        arg_text = "({})".format(', '.join(args))
    output_defbegin(output)
    if doc:
        output_def(output, "#define ", name, arg_text, link)
        output_doc(output, doc)
    else:
        if single_line_expansion and single_line_expansion[-1] != '\\':
            output_def(output, "#define ", name, arg_text + ' ' + single_line_expansion, link)
        else:
            if single_line_expansion:
                print_warning("complex macro without a comment: {}".format(name), link)
            output_def(output, "#define ", name, arg_text, link)
    output_defend(output)

def output_function(output, name, ret_type, args, doc, link):
    if name == 'STR':
        if not doc:
            return
        args = ['...']
    output_defbegin(output)
    output_def(output, ret_type + " ", name, "(" + ", ".join(args) + ");", link)
    output_doc(output, doc)
    output_defend(output)

re_comma = re.compile(''',\s*''')
def output_statement(output, line, doc, link):
    if line.startswith('#define'):
        m = re.match('''^#define\s+(\w+)(\(.*?\))?\s*(.*)''', line)
        assert m
        name = m.group(1)
        if ignore_name(name) or name.endswith('_H'):
            return
        args = None
        if m.group(2):
            args = re.split(re_comma, m.group(2)[1:-1])
        single_line_expansion = m.group(3)
        output_macro(output, name, args, single_line_expansion, doc, link)
        return
    if line.startswith('typedef '):
        m = re.match('''^typedef (.* )(\w+)( __attribute__\(.*\))?;''', line)
        if not m:
            # function pointers
            m = re.match('''^typedef (.* \(\*)(\w+)(\)\(.*\));''', line)
        if not m:
            return ignore_bad_line(line, link)
        before = m.group(1)
        name = m.group(2)
        after = m.group(3) or ''
        output_typedef(output, name, before, after, doc, link)
        return
    if re.match('''^(#|/| \*|extern |static const union |__attribute__\(\(.*\)\)$|CASSERT|(list|dict)\(\w+\);|decl_fid_t)''', line):
        return # ignore cruft
    m = re.match('''^(?:__attribute__\(\(.*?\)\) )?(?:static inline )?((?:const )?[^ ]+) +(\w+)\s*\((.*)\)( NO_NULL_ARGS)?(;| {.*)$''', line)
    if not m:
        return ignore_bad_line(line, link)
    ret_type = m.group(1)
    fn_name = m.group(2)
    args = re.split(re_comma, m.group(3))
    if not ignore_name(fn_name):
        output_function(output, fn_name, ret_type, args, doc, link)

def output_file(output, source_url, lines):
    doc = []
    continuation = False
    st = None
    st_doc = None
    st_isstruct = None
    st_link = None
    infunc = False
    re_comment = re.compile('''^\s*/// ?(.*)''')
    lineno = 0
    for line in lines:
        lineno += 1
        link = source_url + "#L" + str(lineno)
        if infunc:
            if line.startswith('}'):
                infunc = False
            continue
        if not continuation:
            m_comment = re_comment.match(line)
            if not line or ignore_line(line):
                pass
            elif m_comment:
                doc.append(m_comment.group(1))
            elif st is not None:
                if line.startswith('}'):
                    m = re.match('''^} (\w+);''', line)
                    assert m
                    name = m.group(1)
                    if not ignore_name(name):
                        if st_isstruct:
                            output_struct(output, name, st_doc, st_link, st)
                        else:
                            output_enum(output, name, st_doc, st_link, st)
                    st = None
                else:
                    st.append((line, doc, link))
                    doc = []
            elif (line.startswith('typedef struct') or line.startswith('typedef enum')) and line.endswith('{'):
                assert '}' not in line
                st_doc = doc
                st_link = link
                doc = []
                st = []
                st_isstruct = (line.startswith('typedef struct'))
            else:
                output_statement(output, line, doc, link)
                doc = []
        infunc = st is None and not continuation and line.endswith('{')
        continuation = line.endswith('\\')

def doc_file(revision, filename, outputfile, url):
    header_name = os.path.basename(filename)
    output = []
    with open(filename, 'r') as f:
        with open(outputfile, 'w') as fw:
            lines = f.read().split('\n')
            output_header(output, header_name, revision)
            output_file(output, url, lines)
            for line in output:
                fw.write(line + "\n")

def main():
    if len(sys.argv) != 3 or not sys.argv[2].endswith('docs/'):
        sys.stderr.write("usage: {} path/to/includes/ path/to/docs/\n".format(sys.argv[0]))
        sys.exit(1)
    inputdir = sys.argv[1]
    outputdir = sys.argv[2]
    excluded = ['avl.h', 'linux.h', 'musl.h', 'setjmp.h', 'rcd.s', 'utlist.h']
    (_, _, filenames) = next(os.walk(inputdir))
    to_document = [name for name in filenames if name not in excluded]
    revision = subprocess.Popen("git rev-parse HEAD", cwd=inputdir, shell=True, stdout=subprocess.PIPE).stdout.read().strip().decode('utf-8')
    base_url = "https://github.com/jumpstarter-io/librcd/blob/{}/include/".format(revision)
    for f in glob.glob(os.path.join(outputdir, '*.h.html')):
        os.remove(f)
    for name in to_document:
        doc_file(revision, os.path.join(inputdir, name), os.path.join(outputdir, name + '.html'), base_url + name)

main()
