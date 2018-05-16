# Copyright (c) 2018 Jan Brittenson
# See LICENSE for details.

import sys
import grammar, nodes, cppemitter

emit_decls = False
emit_defs = False

body = " "
for arg in sys.argv[1:]:
    if arg == "--decls":
        emit_decls = True
    elif arg == "--defs":
        emit_defs = True
    else:
        with open(arg, 'r') as f:
            body += f.read() + "\n"

n = nodes.Nodes()
grammar.parse(body, actions=n)

n.validate()

emitter = cppemitter.CppEmitter(n.get_nodes(), n.get_palette(), n.get_fonts(), n.get_colors())

if emit_decls:
    emitter.output_decls()

if emit_defs:
    emitter.output_defs()
