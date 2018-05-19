# Copyright (c) 2018 Jan Brittenson
# See LICENSE for details.

import pprint

class CppEmitter:
    def __init__(self, nodes):
        self.nodes = nodes.get_nodes()
        self.palette = nodes.get_palette()
        self.fonts = nodes.get_fonts()
        self.colors = nodes.get_colors()
        self.consts = nodes.get_consts()
        self.funcs = nodes.get_funcs()

    def def_palette(self):
        print "\nconst uint32_t _palette[%d] = {" % len(self.palette)
        print "  ", ", ".join(map(lambda rgb: "0x%06x" % rgb, self.palette))
        print "};"

    def decl_palette(self):
        print "extern const uint32_t _palette[%d];" % len(self.palette)

    def decl_consts(self):
        print "enum {"
        for const in self.consts:
            print "    %s = %s," % (const, self.consts[const])
        print "};"

    def decl_funcs(self):
        for f in self.funcs:
            print "void %s(uint32_t);" % f

    def decl_node(self, node):
        print "extern ui::%s %s;" % (node['type'].capitalize(), node['id'])

    def labelconf(self, node):
        return "{%s, %s}, %s, %s, &_font_%s, %s, %s" % (node['size'][0], node['size'][1], node['bg'], node['fg'], node['font'], node['tap'][0], node['tap'][1])

    def hlineconf(self, node):
        return "{%s, %s}, %s" % (node['size'][0], node['size'][1], node['fg'])

    def vlineconf(self, node):
        return "{%s, %s}, %s" % (node['size'][0], node['size'][1], node['fg'])

    def indicatorconf(self, node):
        return "%s, %s, &_font_%s, %s, %s, %s, %s" % (node['bg'], node['fg'], node['font'], node['true'], node['false'], node['tap'][0], node['tap'][1])

    def integerconf(self, node):
        return "{%s, %s}, %s, %s, &_font_%s, %s, %s, %s" % (node['size'][0], node['size'][1], node['bg'], node['fg'], node['font'], node['fmt'], node['tap'][0], node['tap'][1])

    def windowconf(self, node):
        children = node['children']
        # Make sure all children window references are emitted first
        for child in children:
            child_node = self.nodes[child['id']]
            if not 'emitted' in child_node:
                self.def_node(child_node)

        s = "{%s, %s}, %s, %s, %s," % (node['width'], node['height'], node['bg'], node['fg'], len(node['children']))
        c = ""
        for child in children:
            if c != "":
                c += ",\n     "
            else:
                c += "    ["
            c += "{ %s, %s, &%s, &%s_ro }" % (child['x'], child['y'], child['id'], child['id'])
        s += "\n" + c + "]\n"
        return s

    CONF_FUNCS = {
        'window': windowconf,
        'integer': integerconf,
        'indicator': indicatorconf,
        'vline': vlineconf,
        'hline': hlineconf,
        'label': labelconf
    }

    def def_node(self, node):
        # Avoid emitting twice
        if 'emitted' in node:
            return

        func = self.CONF_FUNCS[node['type']]
        print "\nstatic const struct ui::%s::Config %s_ro = {%s};" % (node['type'].capitalize(), node['id'], func(self, node))

        print "ui::%s %s;" % (node['type'].capitalize(), node['id'])
        node['emitted'] = True

    def decl_nodes(self):
        print "// UI elements"
        for name in self.nodes:
            self.decl_node(self.nodes[name])

    def def_nodes(self):
        print "// UI elements"
        for name in self.nodes:
            node = self.nodes[name]
            if not 'emitted' in node:
                self.def_node(node)

    def output_decls(self):
        print "#include <stdint.h>"
        print "#include \"ui.h\""
        print "\nnamespace uibuilder {"

        self.decl_palette()
        print
        self.decl_consts()
        print
        self.decl_nodes()

        print "}; // namespace uibuilder"

        print
        self.decl_funcs()

    def output_defs(self):
        print "#include <stdint.h>"
        print "#include \"ui.h\""
        print "\nnamespace uibuilder {"

        self.def_palette()
        self.def_nodes()

        print
        print "}; // namespace uibuilder"


    def debug(self):
        pp = pprint.PrettyPrinter(indent=4)

        print "nodes:"
        pp.pprint(self.nodes)

        print
        print "colors:"
        pp.pprint(self.colors)

        print
        print "fonts:"
        pp.pprint(self.fonts)