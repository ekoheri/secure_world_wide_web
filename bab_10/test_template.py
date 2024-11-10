from template_engine import \
    parse_template, \
    compile_template

fl = open('dokumen/template.html','r')
template_text = fl.read()
fl.close()

qstring ="nama=eko heri&alamat=malang"

# Kompilasi template
tokens = parse_template(template_text)

output = compile_template(tokens, query_string=qstring)
print(output)