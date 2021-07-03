# Copyright (c) 2018 Jan Brittenson
# See LICENSE for details.

class Nodes(object):

    def __init__(self):
        self.color_aliases = { }
        self.font_aliases = { }
        self.id_count = 0
        self.nodes = { }
        self.palette = [ ]
        self.consts = { }
        self.funcs = { }
        
    def new_identifier(self):
        self.id_count = self.id_count + 1
        return "_ui_%s" % self.id_count

    def get_identifier(self, element):
        texts = element.text
        if texts == "":
            return self.new_identifier()
        texts = texts.strip().split()[0]
        if texts[-1] == "=":
            texts = texts[:-1]
        return texts
        
    def color(self, color):
        if color in self.color_aliases:
            return self.color_aliases[color]
        print "Undefined color %s" % color
        return 0

    def font(self, font):
        if font in self.font_aliases:
            return self.font_aliases[font]
        print "Undefined font %s" % font
        return font

    def size(self, sizestr):
        return map(int, sizestr.split("x"))

    def func(self, element):
        s = element.text
        if s == "*" or s == "":
            self.funcs["NoTap"] = True
            return [ "NoTap", 0 ]

        f = element.cidentifier.text
        arg = element.arg.text
        if arg == "":
            arg = 0
        self.funcs[f] = True
        return [ f, arg ]

    def const(self, element):
        s = element.text
        if s in self.consts:
            return s

        return int(s)

    def to_rgb(self, s):
        rgb = s.split('/');
        return (int(rgb[0]) << 16) + (int(rgb[1]) << 8) + int(rgb[2])

    def set_palette(self, input, start, end, elements):
        self.palette = map(self.to_rgb, elements[2].text.split())
        return { 'type': 'palette',
                 'values': self.palette }

    def make_color_alias(self, input, start, end, elements):
        self.color_aliases[elements[2].text] = int(elements[4].text)
        return None

    def make_font_alias(self, input, start, end, elements):
        self.font_aliases[elements[2].text] = elements[4].text[1:-1]
        return None

    def make_const(self, input, start, end, elements):
        self.consts[elements[2].text] = int(elements[4].text)
        return None
    
    def make_label(self, input, start, end, elements):
        identifier = self.get_identifier(elements[2])
        node = { 'type': 'label',
                 'text': elements[3].text[1:-1],
                 'font': self.font(elements[5].text),
                 'fg': self.color(elements[7].text),
                 'bg': self.color(elements[9].text),
                 'size': self.size(elements[11].text),
                 'tap': self.func(elements[13]),
                 'id': identifier }
        self.nodes[identifier] = node;
        return node;

    def make_button(self, input, start, end, elements):
        identifier = self.get_identifier(elements[2])
        node = { 'type': 'button',
                 'text': elements[3].text[1:-1],
                 'font': self.font(elements[5].text),
                 'fg': self.color(elements[7].text),
                 'bg': self.color(elements[9].text),
                 'size': self.size(elements[11].text),
                 'indent': self.const(elements[13]),
                 'tap': self.func(elements[15]),
                 'id': identifier }
        self.nodes[identifier] = node;
        return node;

    def make_hline(self, input, start, end, elements):
        identifier = self.get_identifier(elements[2])
        node = { 'type': 'hline',
                 'size': self.size(elements[3].text),
                 'fg': self.color(elements[5].text),
                 'id': identifier }
        self.nodes[identifier] = node;
        return node;

    def make_vline(self, input, start, end, elements):
        identifier = self.get_identifier(elements[2])
        node = { 'type': 'vline',
                 'size': self.size(elements[3].text),
                 'fg': self.color(elements[5].text),
                 'id': identifier }
        self.nodes[identifier] = node;
        return node;

    def make_frame(self, input, start, end, elements):
        identifier = self.get_identifier(elements[2])
        node = { 'type': 'frame',
                 'size': self.size(elements[3].text),
                 'width': self.const(elements[5]),
                 'fg': self.color(elements[7].text),
                 'id': identifier }
        self.nodes[identifier] = node;
        return node;

    def make_indicator(self, input, start, end, elements):
        identifier = self.get_identifier(elements[2])
        node = { 'type': 'indicator',
                 'font': self.font(elements[3].text),
                 'fg': self.color(elements[5].text),
                 'bg': self.color(elements[7].text),
                 'true': int(elements[9].text),
                 'false': int(elements[11].text),
                 'tap': self.func(elements[13]),
                 'id': identifier }
        self.nodes[identifier] = node;
        return node;
        
    def make_integer(self, input, start, end, elements):
        identifier = self.get_identifier(elements[2])
        node = { 'type': 'integer',
                 'font': self.font(elements[3].text),
                 'size': self.size(elements[5].text),
                 'fg': self.color(elements[7].text),
                 'bg': self.color(elements[9].text),
                 'fmt': elements[11].text,
                 'tap': self.func(elements[13]),
                 'id': identifier }
        self.nodes[identifier] = node;
        return node;

    def child_map(self, treenode):
        value = { 'x': int(treenode.x.text),
                  'y': int(treenode.y.text),
                  'id': None }
        if hasattr(treenode, 'child'):
            child = treenode.child
            if hasattr(child, 'identifier'):
                value['id'] = child.identifier.text
            if hasattr(child, 'definition'):
                value['id'] = child.definition['id']

        return value

    def make_window(self, input, start, end, elements):
        identifier = self.get_identifier(elements[2])
        children = map(self.child_map, elements[11])
        node = { 'type': 'window',
                 'width': int(elements[3].text),
                 'height': int(elements[5].text),
                 'fg': self.color(elements[7].text),
                 'bg': self.color(elements[9].text),
                 'children': children,
                 'id': identifier }
        self.nodes[identifier] = node;
        return node;
        
    def make_view(self, input, start, end, elements):
        identifier = self.get_identifier(elements[2])
        children = map(self.child_map, elements[11])
        node = { 'type': 'view',
                 'width': int(elements[3].text),
                 'height': int(elements[5].text),
                 'fg': self.color(elements[7].text),
                 'bg': self.color(elements[9].text),
                 'children': children,
                 'id': identifier }
        self.nodes[identifier] = node;
        return node;
        
    def get_nodes(self):
        return self.nodes

    def get_colors(self):
        return self.color_aliases

    def get_fonts(self):
        return self.font_aliases

    def get_palette(self):
        return self.palette
    
    def get_consts(self):
        return self.consts

    def get_funcs(self):
        return self.funcs

    # Run checks to make sure things look alright
    def validate(self):
        for k in self.color_aliases:
            v = self.color_aliases[k]
            if v < 0 or v >= len(self.palette):
                print "Color %s has index %s outside palette of size %s" % (k, v, len(self.palette))
        for k in self.nodes:
            v = self.nodes[k]
            if 'fg' in v:
                if v['fg'] >= len(self.palette):
                    print "Undefined color %s in %s %s" % (v['fg'], v['type'], v['id'])
            if 'bg' in v:
                if v['bg'] >= len(self.palette):
                    print "Undefined color %s in %s %s" % (v['bg'], v['type'], v['id'])
    
