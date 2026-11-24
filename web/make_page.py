#!/usr/bin/env python3
"""Assemble arcturus.html: page.html with the wasm, the run-time Ada
files, wasi.js and the examples inlined, so the page is one file."""
import base64, json, os, sys
here = os.path.dirname(os.path.abspath(__file__))
port = os.path.join(here, "..", "port")
wasm = sys.argv[1] if len(sys.argv) > 1 else os.path.join(here, "arcturus.wasm")
page = open(os.path.join(here, "page.html")).read()
lib = {"/lib/" + f: open(os.path.join(port, "lib", f), encoding="latin-1").read()
       for f in sorted(os.listdir(os.path.join(port, "lib")))}
examples = []
for name, path in [("Tour", "tour.ada"), ("Hello", "hello.ada")]:
    examples.append({"name": name, "text": open(os.path.join(port, "examples", path)).read().rstrip("\n")})
for f in sorted(os.listdir(os.path.join(here, "examples"))):
    text = open(os.path.join(here, "examples", f)).read()
    first, rest = text.split("\n", 1)
    examples.append({"name": first.lstrip("- ").strip(), "text": rest.rstrip("\n")})
def js(o):  # safe inside <script>
    return json.dumps(o).replace("</", "<\\/")
page = (page.replace("@@WASM@@", base64.b64encode(open(wasm, "rb").read()).decode())
            .replace("@@LIB@@", js(lib))
            .replace("@@WASI@@", open(os.path.join(here, "wasi.js")).read().replace("</", "<\\/"))
            .replace("@@EXAMPLES@@", js(examples)))
open(os.path.join(here, "arcturus.html"), "w").write(page)
print("arcturus.html", len(page), "bytes")
