import sys
import grammar, nodes
import pprint

body = " "
for file in sys.argv[1:]:
    with open(file, 'r') as f:
        body += f.read() + "\n"

n = nodes.Nodes()
grammar.parse(body, actions=n)

pp = pprint.PrettyPrinter(indent=4)

print "nodes:"
pp.pprint(n.get_nodes())

print
print "colors:"
pp.pprint(n.get_colors())

print
print "fonts:"
pp.pprint(n.get_fonts())

print
print "palette:"
pp.pprint(n.get_palette())

n.validate()
