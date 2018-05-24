from collections import defaultdict
import re


class TreeNode(object):
    def __init__(self, text, offset, elements=None):
        self.text = text
        self.offset = offset
        self.elements = elements or []

    def __iter__(self):
        for el in self.elements:
            yield el


class TreeNode1(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode1, self).__init__(text, offset, elements)
        self.SP = elements[1]


class TreeNode2(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode2, self).__init__(text, offset, elements)
        self.rgb = elements[0]
        self.SP = elements[1]


class TreeNode3(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode3, self).__init__(text, offset, elements)
        self.SP = elements[5]
        self.identifier = elements[2]
        self.EQUALS = elements[3]
        self.integer = elements[4]


class TreeNode4(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode4, self).__init__(text, offset, elements)
        self.SP = elements[5]
        self.identifier = elements[2]
        self.EQUALS = elements[3]
        self.string = elements[4]


class TreeNode5(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode5, self).__init__(text, offset, elements)
        self.SP = elements[5]
        self.identifier = elements[2]
        self.EQUALS = elements[3]
        self.integer = elements[4]


class TreeNode6(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode6, self).__init__(text, offset, elements)
        self.SP = elements[14]
        self.text = elements[3]
        self.string = elements[3]
        self.COMMA = elements[12]
        self.font = elements[5]
        self.identifier = elements[5]
        self.fg = elements[7]
        self.color = elements[9]
        self.bg = elements[9]
        self.size = elements[11]
        self.SIZE = elements[11]
        self.tap = elements[13]
        self.FUNC = elements[13]


class TreeNode7(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode7, self).__init__(text, offset, elements)
        self.id = elements[0]
        self.identifier = elements[0]
        self.EQUALS = elements[1]


class TreeNode8(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode8, self).__init__(text, offset, elements)
        self.SP = elements[16]
        self.text = elements[3]
        self.string = elements[3]
        self.COMMA = elements[14]
        self.font = elements[5]
        self.identifier = elements[5]
        self.fg = elements[7]
        self.color = elements[9]
        self.bg = elements[9]
        self.size = elements[11]
        self.SIZE = elements[11]
        self.indent = elements[13]
        self.const = elements[13]
        self.tap = elements[15]
        self.FUNC = elements[15]


class TreeNode9(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode9, self).__init__(text, offset, elements)
        self.id = elements[0]
        self.identifier = elements[0]
        self.EQUALS = elements[1]


class TreeNode10(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode10, self).__init__(text, offset, elements)
        self.SP = elements[6]
        self.size = elements[3]
        self.SIZE = elements[3]
        self.COMMA = elements[4]
        self.fg = elements[5]
        self.color = elements[5]


class TreeNode11(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode11, self).__init__(text, offset, elements)
        self.id = elements[0]
        self.identifier = elements[0]
        self.EQUALS = elements[1]


class TreeNode12(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode12, self).__init__(text, offset, elements)
        self.SP = elements[6]
        self.size = elements[3]
        self.SIZE = elements[3]
        self.COMMA = elements[4]
        self.fg = elements[5]
        self.color = elements[5]


class TreeNode13(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode13, self).__init__(text, offset, elements)
        self.id = elements[0]
        self.identifier = elements[0]
        self.EQUALS = elements[1]


class TreeNode14(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode14, self).__init__(text, offset, elements)
        self.SP = elements[14]
        self.font = elements[3]
        self.identifier = elements[3]
        self.COMMA = elements[12]
        self.fg = elements[5]
        self.color = elements[7]
        self.bg = elements[7]
        self.true = elements[9]
        self.integer = elements[11]
        self.false = elements[11]
        self.tap = elements[13]
        self.FUNC = elements[13]


class TreeNode15(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode15, self).__init__(text, offset, elements)
        self.id = elements[0]
        self.identifier = elements[0]
        self.EQUALS = elements[1]


class TreeNode16(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode16, self).__init__(text, offset, elements)
        self.SP = elements[14]
        self.font = elements[3]
        self.identifier = elements[3]
        self.COMMA = elements[12]
        self.size = elements[5]
        self.SIZE = elements[5]
        self.fg = elements[7]
        self.color = elements[9]
        self.bg = elements[9]
        self.fmt = elements[11]
        self.string = elements[11]
        self.tap = elements[13]
        self.FUNC = elements[13]


class TreeNode17(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode17, self).__init__(text, offset, elements)
        self.id = elements[0]
        self.identifier = elements[0]
        self.EQUALS = elements[1]


class TreeNode18(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode18, self).__init__(text, offset, elements)
        self.SP = elements[12]
        self.width = elements[3]
        self.integer = elements[5]
        self.height = elements[5]
        self.COMMA = elements[8]
        self.fg = elements[7]
        self.color = elements[9]
        self.bg = elements[9]
        self.children = elements[11]


class TreeNode19(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode19, self).__init__(text, offset, elements)
        self.id = elements[0]
        self.identifier = elements[0]
        self.EQUALS = elements[1]


class TreeNode20(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode20, self).__init__(text, offset, elements)
        self.AT = elements[0]
        self.x = elements[1]
        self.integer = elements[3]
        self.COMMA = elements[2]
        self.y = elements[3]
        self.COLON = elements[4]
        self.child = elements[5]


class TreeNode21(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode21, self).__init__(text, offset, elements)
        self.SP = elements[0]
        self.value = elements[1]
        self.definition = elements[1]


class TreeNode22(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode22, self).__init__(text, offset, elements)
        self.id = elements[0]
        self.identifier = elements[0]
        self.SP = elements[1]


class TreeNode23(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode23, self).__init__(text, offset, elements)
        self.identifier = elements[1]


class TreeNode24(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode24, self).__init__(text, offset, elements)
        self.SP = elements[2]


class TreeNode25(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode25, self).__init__(text, offset, elements)
        self.SP = elements[2]


class TreeNode26(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode26, self).__init__(text, offset, elements)
        self.SP = elements[2]


class TreeNode27(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode27, self).__init__(text, offset, elements)
        self.SP = elements[2]


class TreeNode28(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode28, self).__init__(text, offset, elements)
        self.integer = elements[4]


class TreeNode29(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode29, self).__init__(text, offset, elements)
        self.integer = elements[2]


class TreeNode30(TreeNode):
    def __init__(self, text, offset, elements):
        super(TreeNode30, self).__init__(text, offset, elements)
        self.cidentifier = elements[0]
        self.arg = elements[2]


class ParseError(SyntaxError):
    pass


FAILURE = object()


class Grammar(object):
    REGEX_1 = re.compile('^[0-9]')
    REGEX_2 = re.compile('^[a-zA-Z_]')
    REGEX_3 = re.compile('^[a-zA-Z0-9_.]')
    REGEX_4 = re.compile('^[^"]')
    REGEX_5 = re.compile('^[^\\n]')

    def _read_input(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['input'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        remaining0, index1, elements0, address1 = 0, self._offset, [], True
        while address1 is not FAILURE:
            address1 = self._read_statement()
            if address1 is not FAILURE:
                elements0.append(address1)
                remaining0 -= 1
        if remaining0 <= 0:
            address0 = TreeNode(self._input[index1:self._offset], index1, elements0)
            self._offset = self._offset
        else:
            address0 = FAILURE
        self._cache['input'][index0] = (address0, self._offset)
        return address0

    def _read_definition(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['definition'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1 = self._offset
        address0 = self._read_labeldef()
        if address0 is FAILURE:
            self._offset = index1
            address0 = self._read_hlinedef()
            if address0 is FAILURE:
                self._offset = index1
                address0 = self._read_vlinedef()
                if address0 is FAILURE:
                    self._offset = index1
                    address0 = self._read_indicatordef()
                    if address0 is FAILURE:
                        self._offset = index1
                        address0 = self._read_integerdef()
                        if address0 is FAILURE:
                            self._offset = index1
                            address0 = self._read_windowdef()
                            if address0 is FAILURE:
                                self._offset = index1
                                address0 = self._read_buttondef()
                                if address0 is FAILURE:
                                    self._offset = index1
        self._cache['definition'][index0] = (address0, self._offset)
        return address0

    def _read_statement(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['statement'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1 = self._offset
        address0 = self._read_comment()
        if address0 is FAILURE:
            self._offset = index1
            address0 = self._read_color_alias()
            if address0 is FAILURE:
                self._offset = index1
                address0 = self._read_font_alias()
                if address0 is FAILURE:
                    self._offset = index1
                    address0 = self._read_const_alias()
                    if address0 is FAILURE:
                        self._offset = index1
                        address0 = self._read_palette()
                        if address0 is FAILURE:
                            self._offset = index1
                            address0 = self._read_definition()
                            if address0 is FAILURE:
                                self._offset = index1
                                remaining0, index2, elements0, address1 = 1, self._offset, [], True
                                while address1 is not FAILURE:
                                    chunk0 = None
                                    if self._offset < self._input_size:
                                        chunk0 = self._input[self._offset:self._offset + 1]
                                    if chunk0 == ' ':
                                        address1 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                        self._offset = self._offset + 1
                                    else:
                                        address1 = FAILURE
                                        if self._offset > self._failure:
                                            self._failure = self._offset
                                            self._expected = []
                                        if self._offset == self._failure:
                                            self._expected.append('" "')
                                    if address1 is not FAILURE:
                                        elements0.append(address1)
                                        remaining0 -= 1
                                if remaining0 <= 0:
                                    address0 = TreeNode(self._input[index2:self._offset], index2, elements0)
                                    self._offset = self._offset
                                else:
                                    address0 = FAILURE
                                if address0 is FAILURE:
                                    self._offset = index1
                                    remaining1, index3, elements1, address2 = 1, self._offset, [], True
                                    while address2 is not FAILURE:
                                        chunk1 = None
                                        if self._offset < self._input_size:
                                            chunk1 = self._input[self._offset:self._offset + 1]
                                        if chunk1 == '\n':
                                            address2 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                            self._offset = self._offset + 1
                                        else:
                                            address2 = FAILURE
                                            if self._offset > self._failure:
                                                self._failure = self._offset
                                                self._expected = []
                                            if self._offset == self._failure:
                                                self._expected.append('"\\n"')
                                        if address2 is not FAILURE:
                                            elements1.append(address2)
                                            remaining1 -= 1
                                    if remaining1 <= 0:
                                        address0 = TreeNode(self._input[index3:self._offset], index3, elements1)
                                        self._offset = self._offset
                                    else:
                                        address0 = FAILURE
                                    if address0 is FAILURE:
                                        self._offset = index1
        self._cache['statement'][index0] = (address0, self._offset)
        return address0

    def _read_palette(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['palette'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        chunk0 = None
        if self._offset < self._input_size:
            chunk0 = self._input[self._offset:self._offset + 7]
        if chunk0 == 'PALETTE':
            address1 = TreeNode(self._input[self._offset:self._offset + 7], self._offset)
            self._offset = self._offset + 7
        else:
            address1 = FAILURE
            if self._offset > self._failure:
                self._failure = self._offset
                self._expected = []
            if self._offset == self._failure:
                self._expected.append('"PALETTE"')
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            address2 = self._read_SP()
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                remaining0, index2, elements1, address4 = 1, self._offset, [], True
                while address4 is not FAILURE:
                    index3, elements2 = self._offset, []
                    address5 = FAILURE
                    address5 = self._read_rgb()
                    if address5 is not FAILURE:
                        elements2.append(address5)
                        address6 = FAILURE
                        address6 = self._read_SP()
                        if address6 is not FAILURE:
                            elements2.append(address6)
                        else:
                            elements2 = None
                            self._offset = index3
                    else:
                        elements2 = None
                        self._offset = index3
                    if elements2 is None:
                        address4 = FAILURE
                    else:
                        address4 = TreeNode2(self._input[index3:self._offset], index3, elements2)
                        self._offset = self._offset
                    if address4 is not FAILURE:
                        elements1.append(address4)
                        remaining0 -= 1
                if remaining0 <= 0:
                    address3 = TreeNode(self._input[index2:self._offset], index2, elements1)
                    self._offset = self._offset
                else:
                    address3 = FAILURE
                if address3 is not FAILURE:
                    elements0.append(address3)
                    address7 = FAILURE
                    chunk1 = None
                    if self._offset < self._input_size:
                        chunk1 = self._input[self._offset:self._offset + 1]
                    if chunk1 == ';':
                        address7 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                        self._offset = self._offset + 1
                    else:
                        address7 = FAILURE
                        if self._offset > self._failure:
                            self._failure = self._offset
                            self._expected = []
                        if self._offset == self._failure:
                            self._expected.append('";"')
                    if address7 is not FAILURE:
                        elements0.append(address7)
                    else:
                        elements0 = None
                        self._offset = index1
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = self._actions.set_palette(self._input, index1, self._offset, elements0)
            self._offset = self._offset
        self._cache['palette'][index0] = (address0, self._offset)
        return address0

    def _read_color_alias(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['color_alias'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        chunk0 = None
        if self._offset < self._input_size:
            chunk0 = self._input[self._offset:self._offset + 5]
        if chunk0 == 'COLOR':
            address1 = TreeNode(self._input[self._offset:self._offset + 5], self._offset)
            self._offset = self._offset + 5
        else:
            address1 = FAILURE
            if self._offset > self._failure:
                self._failure = self._offset
                self._expected = []
            if self._offset == self._failure:
                self._expected.append('"COLOR"')
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            address2 = self._read_SP()
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                address3 = self._read_identifier()
                if address3 is not FAILURE:
                    elements0.append(address3)
                    address4 = FAILURE
                    address4 = self._read_EQUALS()
                    if address4 is not FAILURE:
                        elements0.append(address4)
                        address5 = FAILURE
                        address5 = self._read_integer()
                        if address5 is not FAILURE:
                            elements0.append(address5)
                            address6 = FAILURE
                            address6 = self._read_SP()
                            if address6 is not FAILURE:
                                elements0.append(address6)
                                address7 = FAILURE
                                chunk1 = None
                                if self._offset < self._input_size:
                                    chunk1 = self._input[self._offset:self._offset + 1]
                                if chunk1 == ';':
                                    address7 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                    self._offset = self._offset + 1
                                else:
                                    address7 = FAILURE
                                    if self._offset > self._failure:
                                        self._failure = self._offset
                                        self._expected = []
                                    if self._offset == self._failure:
                                        self._expected.append('";"')
                                if address7 is not FAILURE:
                                    elements0.append(address7)
                                else:
                                    elements0 = None
                                    self._offset = index1
                            else:
                                elements0 = None
                                self._offset = index1
                        else:
                            elements0 = None
                            self._offset = index1
                    else:
                        elements0 = None
                        self._offset = index1
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = self._actions.make_color_alias(self._input, index1, self._offset, elements0)
            self._offset = self._offset
        self._cache['color_alias'][index0] = (address0, self._offset)
        return address0

    def _read_font_alias(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['font_alias'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        chunk0 = None
        if self._offset < self._input_size:
            chunk0 = self._input[self._offset:self._offset + 4]
        if chunk0 == 'FONT':
            address1 = TreeNode(self._input[self._offset:self._offset + 4], self._offset)
            self._offset = self._offset + 4
        else:
            address1 = FAILURE
            if self._offset > self._failure:
                self._failure = self._offset
                self._expected = []
            if self._offset == self._failure:
                self._expected.append('"FONT"')
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            address2 = self._read_SP()
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                address3 = self._read_identifier()
                if address3 is not FAILURE:
                    elements0.append(address3)
                    address4 = FAILURE
                    address4 = self._read_EQUALS()
                    if address4 is not FAILURE:
                        elements0.append(address4)
                        address5 = FAILURE
                        address5 = self._read_string()
                        if address5 is not FAILURE:
                            elements0.append(address5)
                            address6 = FAILURE
                            address6 = self._read_SP()
                            if address6 is not FAILURE:
                                elements0.append(address6)
                                address7 = FAILURE
                                chunk1 = None
                                if self._offset < self._input_size:
                                    chunk1 = self._input[self._offset:self._offset + 1]
                                if chunk1 == ';':
                                    address7 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                    self._offset = self._offset + 1
                                else:
                                    address7 = FAILURE
                                    if self._offset > self._failure:
                                        self._failure = self._offset
                                        self._expected = []
                                    if self._offset == self._failure:
                                        self._expected.append('";"')
                                if address7 is not FAILURE:
                                    elements0.append(address7)
                                else:
                                    elements0 = None
                                    self._offset = index1
                            else:
                                elements0 = None
                                self._offset = index1
                        else:
                            elements0 = None
                            self._offset = index1
                    else:
                        elements0 = None
                        self._offset = index1
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = self._actions.make_font_alias(self._input, index1, self._offset, elements0)
            self._offset = self._offset
        self._cache['font_alias'][index0] = (address0, self._offset)
        return address0

    def _read_const_alias(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['const_alias'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        chunk0 = None
        if self._offset < self._input_size:
            chunk0 = self._input[self._offset:self._offset + 5]
        if chunk0 == 'CONST':
            address1 = TreeNode(self._input[self._offset:self._offset + 5], self._offset)
            self._offset = self._offset + 5
        else:
            address1 = FAILURE
            if self._offset > self._failure:
                self._failure = self._offset
                self._expected = []
            if self._offset == self._failure:
                self._expected.append('"CONST"')
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            address2 = self._read_SP()
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                address3 = self._read_identifier()
                if address3 is not FAILURE:
                    elements0.append(address3)
                    address4 = FAILURE
                    address4 = self._read_EQUALS()
                    if address4 is not FAILURE:
                        elements0.append(address4)
                        address5 = FAILURE
                        address5 = self._read_integer()
                        if address5 is not FAILURE:
                            elements0.append(address5)
                            address6 = FAILURE
                            address6 = self._read_SP()
                            if address6 is not FAILURE:
                                elements0.append(address6)
                                address7 = FAILURE
                                chunk1 = None
                                if self._offset < self._input_size:
                                    chunk1 = self._input[self._offset:self._offset + 1]
                                if chunk1 == ';':
                                    address7 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                    self._offset = self._offset + 1
                                else:
                                    address7 = FAILURE
                                    if self._offset > self._failure:
                                        self._failure = self._offset
                                        self._expected = []
                                    if self._offset == self._failure:
                                        self._expected.append('";"')
                                if address7 is not FAILURE:
                                    elements0.append(address7)
                                else:
                                    elements0 = None
                                    self._offset = index1
                            else:
                                elements0 = None
                                self._offset = index1
                        else:
                            elements0 = None
                            self._offset = index1
                    else:
                        elements0 = None
                        self._offset = index1
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = self._actions.make_const(self._input, index1, self._offset, elements0)
            self._offset = self._offset
        self._cache['const_alias'][index0] = (address0, self._offset)
        return address0

    def _read_color(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['color'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1 = self._offset
        address0 = self._read_integer()
        if address0 is FAILURE:
            self._offset = index1
            address0 = self._read_identifier()
            if address0 is FAILURE:
                self._offset = index1
        self._cache['color'][index0] = (address0, self._offset)
        return address0

    def _read_labeldef(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['labeldef'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        chunk0 = None
        if self._offset < self._input_size:
            chunk0 = self._input[self._offset:self._offset + 5]
        if chunk0 == 'LABEL':
            address1 = TreeNode(self._input[self._offset:self._offset + 5], self._offset)
            self._offset = self._offset + 5
        else:
            address1 = FAILURE
            if self._offset > self._failure:
                self._failure = self._offset
                self._expected = []
            if self._offset == self._failure:
                self._expected.append('"LABEL"')
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            address2 = self._read_SP()
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                index2 = self._offset
                index3, elements1 = self._offset, []
                address4 = FAILURE
                address4 = self._read_identifier()
                if address4 is not FAILURE:
                    elements1.append(address4)
                    address5 = FAILURE
                    address5 = self._read_EQUALS()
                    if address5 is not FAILURE:
                        elements1.append(address5)
                    else:
                        elements1 = None
                        self._offset = index3
                else:
                    elements1 = None
                    self._offset = index3
                if elements1 is None:
                    address3 = FAILURE
                else:
                    address3 = TreeNode7(self._input[index3:self._offset], index3, elements1)
                    self._offset = self._offset
                if address3 is FAILURE:
                    address3 = TreeNode(self._input[index2:index2], index2)
                    self._offset = index2
                if address3 is not FAILURE:
                    elements0.append(address3)
                    address6 = FAILURE
                    address6 = self._read_string()
                    if address6 is not FAILURE:
                        elements0.append(address6)
                        address7 = FAILURE
                        address7 = self._read_COMMA()
                        if address7 is not FAILURE:
                            elements0.append(address7)
                            address8 = FAILURE
                            address8 = self._read_identifier()
                            if address8 is not FAILURE:
                                elements0.append(address8)
                                address9 = FAILURE
                                address9 = self._read_COMMA()
                                if address9 is not FAILURE:
                                    elements0.append(address9)
                                    address10 = FAILURE
                                    address10 = self._read_color()
                                    if address10 is not FAILURE:
                                        elements0.append(address10)
                                        address11 = FAILURE
                                        address11 = self._read_COMMA()
                                        if address11 is not FAILURE:
                                            elements0.append(address11)
                                            address12 = FAILURE
                                            address12 = self._read_color()
                                            if address12 is not FAILURE:
                                                elements0.append(address12)
                                                address13 = FAILURE
                                                address13 = self._read_COMMA()
                                                if address13 is not FAILURE:
                                                    elements0.append(address13)
                                                    address14 = FAILURE
                                                    address14 = self._read_SIZE()
                                                    if address14 is not FAILURE:
                                                        elements0.append(address14)
                                                        address15 = FAILURE
                                                        address15 = self._read_COMMA()
                                                        if address15 is not FAILURE:
                                                            elements0.append(address15)
                                                            address16 = FAILURE
                                                            address16 = self._read_FUNC()
                                                            if address16 is not FAILURE:
                                                                elements0.append(address16)
                                                                address17 = FAILURE
                                                                address17 = self._read_SP()
                                                                if address17 is not FAILURE:
                                                                    elements0.append(address17)
                                                                    address18 = FAILURE
                                                                    chunk1 = None
                                                                    if self._offset < self._input_size:
                                                                        chunk1 = self._input[self._offset:self._offset + 1]
                                                                    if chunk1 == ';':
                                                                        address18 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                                                        self._offset = self._offset + 1
                                                                    else:
                                                                        address18 = FAILURE
                                                                        if self._offset > self._failure:
                                                                            self._failure = self._offset
                                                                            self._expected = []
                                                                        if self._offset == self._failure:
                                                                            self._expected.append('";"')
                                                                    if address18 is not FAILURE:
                                                                        elements0.append(address18)
                                                                    else:
                                                                        elements0 = None
                                                                        self._offset = index1
                                                                else:
                                                                    elements0 = None
                                                                    self._offset = index1
                                                            else:
                                                                elements0 = None
                                                                self._offset = index1
                                                        else:
                                                            elements0 = None
                                                            self._offset = index1
                                                    else:
                                                        elements0 = None
                                                        self._offset = index1
                                                else:
                                                    elements0 = None
                                                    self._offset = index1
                                            else:
                                                elements0 = None
                                                self._offset = index1
                                        else:
                                            elements0 = None
                                            self._offset = index1
                                    else:
                                        elements0 = None
                                        self._offset = index1
                                else:
                                    elements0 = None
                                    self._offset = index1
                            else:
                                elements0 = None
                                self._offset = index1
                        else:
                            elements0 = None
                            self._offset = index1
                    else:
                        elements0 = None
                        self._offset = index1
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = self._actions.make_label(self._input, index1, self._offset, elements0)
            self._offset = self._offset
        self._cache['labeldef'][index0] = (address0, self._offset)
        return address0

    def _read_buttondef(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['buttondef'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        chunk0 = None
        if self._offset < self._input_size:
            chunk0 = self._input[self._offset:self._offset + 6]
        if chunk0 == 'BUTTON':
            address1 = TreeNode(self._input[self._offset:self._offset + 6], self._offset)
            self._offset = self._offset + 6
        else:
            address1 = FAILURE
            if self._offset > self._failure:
                self._failure = self._offset
                self._expected = []
            if self._offset == self._failure:
                self._expected.append('"BUTTON"')
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            address2 = self._read_SP()
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                index2 = self._offset
                index3, elements1 = self._offset, []
                address4 = FAILURE
                address4 = self._read_identifier()
                if address4 is not FAILURE:
                    elements1.append(address4)
                    address5 = FAILURE
                    address5 = self._read_EQUALS()
                    if address5 is not FAILURE:
                        elements1.append(address5)
                    else:
                        elements1 = None
                        self._offset = index3
                else:
                    elements1 = None
                    self._offset = index3
                if elements1 is None:
                    address3 = FAILURE
                else:
                    address3 = TreeNode9(self._input[index3:self._offset], index3, elements1)
                    self._offset = self._offset
                if address3 is FAILURE:
                    address3 = TreeNode(self._input[index2:index2], index2)
                    self._offset = index2
                if address3 is not FAILURE:
                    elements0.append(address3)
                    address6 = FAILURE
                    address6 = self._read_string()
                    if address6 is not FAILURE:
                        elements0.append(address6)
                        address7 = FAILURE
                        address7 = self._read_COMMA()
                        if address7 is not FAILURE:
                            elements0.append(address7)
                            address8 = FAILURE
                            address8 = self._read_identifier()
                            if address8 is not FAILURE:
                                elements0.append(address8)
                                address9 = FAILURE
                                address9 = self._read_COMMA()
                                if address9 is not FAILURE:
                                    elements0.append(address9)
                                    address10 = FAILURE
                                    address10 = self._read_color()
                                    if address10 is not FAILURE:
                                        elements0.append(address10)
                                        address11 = FAILURE
                                        address11 = self._read_COMMA()
                                        if address11 is not FAILURE:
                                            elements0.append(address11)
                                            address12 = FAILURE
                                            address12 = self._read_color()
                                            if address12 is not FAILURE:
                                                elements0.append(address12)
                                                address13 = FAILURE
                                                address13 = self._read_COMMA()
                                                if address13 is not FAILURE:
                                                    elements0.append(address13)
                                                    address14 = FAILURE
                                                    address14 = self._read_SIZE()
                                                    if address14 is not FAILURE:
                                                        elements0.append(address14)
                                                        address15 = FAILURE
                                                        address15 = self._read_COMMA()
                                                        if address15 is not FAILURE:
                                                            elements0.append(address15)
                                                            address16 = FAILURE
                                                            address16 = self._read_const()
                                                            if address16 is not FAILURE:
                                                                elements0.append(address16)
                                                                address17 = FAILURE
                                                                address17 = self._read_COMMA()
                                                                if address17 is not FAILURE:
                                                                    elements0.append(address17)
                                                                    address18 = FAILURE
                                                                    address18 = self._read_FUNC()
                                                                    if address18 is not FAILURE:
                                                                        elements0.append(address18)
                                                                        address19 = FAILURE
                                                                        address19 = self._read_SP()
                                                                        if address19 is not FAILURE:
                                                                            elements0.append(address19)
                                                                            address20 = FAILURE
                                                                            chunk1 = None
                                                                            if self._offset < self._input_size:
                                                                                chunk1 = self._input[self._offset:self._offset + 1]
                                                                            if chunk1 == ';':
                                                                                address20 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                                                                self._offset = self._offset + 1
                                                                            else:
                                                                                address20 = FAILURE
                                                                                if self._offset > self._failure:
                                                                                    self._failure = self._offset
                                                                                    self._expected = []
                                                                                if self._offset == self._failure:
                                                                                    self._expected.append('";"')
                                                                            if address20 is not FAILURE:
                                                                                elements0.append(address20)
                                                                            else:
                                                                                elements0 = None
                                                                                self._offset = index1
                                                                        else:
                                                                            elements0 = None
                                                                            self._offset = index1
                                                                    else:
                                                                        elements0 = None
                                                                        self._offset = index1
                                                                else:
                                                                    elements0 = None
                                                                    self._offset = index1
                                                            else:
                                                                elements0 = None
                                                                self._offset = index1
                                                        else:
                                                            elements0 = None
                                                            self._offset = index1
                                                    else:
                                                        elements0 = None
                                                        self._offset = index1
                                                else:
                                                    elements0 = None
                                                    self._offset = index1
                                            else:
                                                elements0 = None
                                                self._offset = index1
                                        else:
                                            elements0 = None
                                            self._offset = index1
                                    else:
                                        elements0 = None
                                        self._offset = index1
                                else:
                                    elements0 = None
                                    self._offset = index1
                            else:
                                elements0 = None
                                self._offset = index1
                        else:
                            elements0 = None
                            self._offset = index1
                    else:
                        elements0 = None
                        self._offset = index1
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = self._actions.make_button(self._input, index1, self._offset, elements0)
            self._offset = self._offset
        self._cache['buttondef'][index0] = (address0, self._offset)
        return address0

    def _read_hlinedef(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['hlinedef'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        chunk0 = None
        if self._offset < self._input_size:
            chunk0 = self._input[self._offset:self._offset + 5]
        if chunk0 == 'HLINE':
            address1 = TreeNode(self._input[self._offset:self._offset + 5], self._offset)
            self._offset = self._offset + 5
        else:
            address1 = FAILURE
            if self._offset > self._failure:
                self._failure = self._offset
                self._expected = []
            if self._offset == self._failure:
                self._expected.append('"HLINE"')
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            address2 = self._read_SP()
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                index2 = self._offset
                index3, elements1 = self._offset, []
                address4 = FAILURE
                address4 = self._read_identifier()
                if address4 is not FAILURE:
                    elements1.append(address4)
                    address5 = FAILURE
                    address5 = self._read_EQUALS()
                    if address5 is not FAILURE:
                        elements1.append(address5)
                    else:
                        elements1 = None
                        self._offset = index3
                else:
                    elements1 = None
                    self._offset = index3
                if elements1 is None:
                    address3 = FAILURE
                else:
                    address3 = TreeNode11(self._input[index3:self._offset], index3, elements1)
                    self._offset = self._offset
                if address3 is FAILURE:
                    address3 = TreeNode(self._input[index2:index2], index2)
                    self._offset = index2
                if address3 is not FAILURE:
                    elements0.append(address3)
                    address6 = FAILURE
                    address6 = self._read_SIZE()
                    if address6 is not FAILURE:
                        elements0.append(address6)
                        address7 = FAILURE
                        address7 = self._read_COMMA()
                        if address7 is not FAILURE:
                            elements0.append(address7)
                            address8 = FAILURE
                            address8 = self._read_color()
                            if address8 is not FAILURE:
                                elements0.append(address8)
                                address9 = FAILURE
                                address9 = self._read_SP()
                                if address9 is not FAILURE:
                                    elements0.append(address9)
                                    address10 = FAILURE
                                    chunk1 = None
                                    if self._offset < self._input_size:
                                        chunk1 = self._input[self._offset:self._offset + 1]
                                    if chunk1 == ';':
                                        address10 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                        self._offset = self._offset + 1
                                    else:
                                        address10 = FAILURE
                                        if self._offset > self._failure:
                                            self._failure = self._offset
                                            self._expected = []
                                        if self._offset == self._failure:
                                            self._expected.append('";"')
                                    if address10 is not FAILURE:
                                        elements0.append(address10)
                                    else:
                                        elements0 = None
                                        self._offset = index1
                                else:
                                    elements0 = None
                                    self._offset = index1
                            else:
                                elements0 = None
                                self._offset = index1
                        else:
                            elements0 = None
                            self._offset = index1
                    else:
                        elements0 = None
                        self._offset = index1
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = self._actions.make_hline(self._input, index1, self._offset, elements0)
            self._offset = self._offset
        self._cache['hlinedef'][index0] = (address0, self._offset)
        return address0

    def _read_vlinedef(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['vlinedef'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        chunk0 = None
        if self._offset < self._input_size:
            chunk0 = self._input[self._offset:self._offset + 5]
        if chunk0 == 'VLINE':
            address1 = TreeNode(self._input[self._offset:self._offset + 5], self._offset)
            self._offset = self._offset + 5
        else:
            address1 = FAILURE
            if self._offset > self._failure:
                self._failure = self._offset
                self._expected = []
            if self._offset == self._failure:
                self._expected.append('"VLINE"')
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            address2 = self._read_SP()
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                index2 = self._offset
                index3, elements1 = self._offset, []
                address4 = FAILURE
                address4 = self._read_identifier()
                if address4 is not FAILURE:
                    elements1.append(address4)
                    address5 = FAILURE
                    address5 = self._read_EQUALS()
                    if address5 is not FAILURE:
                        elements1.append(address5)
                    else:
                        elements1 = None
                        self._offset = index3
                else:
                    elements1 = None
                    self._offset = index3
                if elements1 is None:
                    address3 = FAILURE
                else:
                    address3 = TreeNode13(self._input[index3:self._offset], index3, elements1)
                    self._offset = self._offset
                if address3 is FAILURE:
                    address3 = TreeNode(self._input[index2:index2], index2)
                    self._offset = index2
                if address3 is not FAILURE:
                    elements0.append(address3)
                    address6 = FAILURE
                    address6 = self._read_SIZE()
                    if address6 is not FAILURE:
                        elements0.append(address6)
                        address7 = FAILURE
                        address7 = self._read_COMMA()
                        if address7 is not FAILURE:
                            elements0.append(address7)
                            address8 = FAILURE
                            address8 = self._read_color()
                            if address8 is not FAILURE:
                                elements0.append(address8)
                                address9 = FAILURE
                                address9 = self._read_SP()
                                if address9 is not FAILURE:
                                    elements0.append(address9)
                                    address10 = FAILURE
                                    chunk1 = None
                                    if self._offset < self._input_size:
                                        chunk1 = self._input[self._offset:self._offset + 1]
                                    if chunk1 == ';':
                                        address10 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                        self._offset = self._offset + 1
                                    else:
                                        address10 = FAILURE
                                        if self._offset > self._failure:
                                            self._failure = self._offset
                                            self._expected = []
                                        if self._offset == self._failure:
                                            self._expected.append('";"')
                                    if address10 is not FAILURE:
                                        elements0.append(address10)
                                    else:
                                        elements0 = None
                                        self._offset = index1
                                else:
                                    elements0 = None
                                    self._offset = index1
                            else:
                                elements0 = None
                                self._offset = index1
                        else:
                            elements0 = None
                            self._offset = index1
                    else:
                        elements0 = None
                        self._offset = index1
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = self._actions.make_vline(self._input, index1, self._offset, elements0)
            self._offset = self._offset
        self._cache['vlinedef'][index0] = (address0, self._offset)
        return address0

    def _read_indicatordef(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['indicatordef'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        chunk0 = None
        if self._offset < self._input_size:
            chunk0 = self._input[self._offset:self._offset + 9]
        if chunk0 == 'INDICATOR':
            address1 = TreeNode(self._input[self._offset:self._offset + 9], self._offset)
            self._offset = self._offset + 9
        else:
            address1 = FAILURE
            if self._offset > self._failure:
                self._failure = self._offset
                self._expected = []
            if self._offset == self._failure:
                self._expected.append('"INDICATOR"')
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            address2 = self._read_SP()
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                index2 = self._offset
                index3, elements1 = self._offset, []
                address4 = FAILURE
                address4 = self._read_identifier()
                if address4 is not FAILURE:
                    elements1.append(address4)
                    address5 = FAILURE
                    address5 = self._read_EQUALS()
                    if address5 is not FAILURE:
                        elements1.append(address5)
                    else:
                        elements1 = None
                        self._offset = index3
                else:
                    elements1 = None
                    self._offset = index3
                if elements1 is None:
                    address3 = FAILURE
                else:
                    address3 = TreeNode15(self._input[index3:self._offset], index3, elements1)
                    self._offset = self._offset
                if address3 is FAILURE:
                    address3 = TreeNode(self._input[index2:index2], index2)
                    self._offset = index2
                if address3 is not FAILURE:
                    elements0.append(address3)
                    address6 = FAILURE
                    address6 = self._read_identifier()
                    if address6 is not FAILURE:
                        elements0.append(address6)
                        address7 = FAILURE
                        address7 = self._read_COMMA()
                        if address7 is not FAILURE:
                            elements0.append(address7)
                            address8 = FAILURE
                            address8 = self._read_color()
                            if address8 is not FAILURE:
                                elements0.append(address8)
                                address9 = FAILURE
                                address9 = self._read_COMMA()
                                if address9 is not FAILURE:
                                    elements0.append(address9)
                                    address10 = FAILURE
                                    address10 = self._read_color()
                                    if address10 is not FAILURE:
                                        elements0.append(address10)
                                        address11 = FAILURE
                                        address11 = self._read_COMMA()
                                        if address11 is not FAILURE:
                                            elements0.append(address11)
                                            address12 = FAILURE
                                            address12 = self._read_integer()
                                            if address12 is not FAILURE:
                                                elements0.append(address12)
                                                address13 = FAILURE
                                                address13 = self._read_COMMA()
                                                if address13 is not FAILURE:
                                                    elements0.append(address13)
                                                    address14 = FAILURE
                                                    address14 = self._read_integer()
                                                    if address14 is not FAILURE:
                                                        elements0.append(address14)
                                                        address15 = FAILURE
                                                        address15 = self._read_COMMA()
                                                        if address15 is not FAILURE:
                                                            elements0.append(address15)
                                                            address16 = FAILURE
                                                            address16 = self._read_FUNC()
                                                            if address16 is not FAILURE:
                                                                elements0.append(address16)
                                                                address17 = FAILURE
                                                                address17 = self._read_SP()
                                                                if address17 is not FAILURE:
                                                                    elements0.append(address17)
                                                                    address18 = FAILURE
                                                                    chunk1 = None
                                                                    if self._offset < self._input_size:
                                                                        chunk1 = self._input[self._offset:self._offset + 1]
                                                                    if chunk1 == ';':
                                                                        address18 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                                                        self._offset = self._offset + 1
                                                                    else:
                                                                        address18 = FAILURE
                                                                        if self._offset > self._failure:
                                                                            self._failure = self._offset
                                                                            self._expected = []
                                                                        if self._offset == self._failure:
                                                                            self._expected.append('";"')
                                                                    if address18 is not FAILURE:
                                                                        elements0.append(address18)
                                                                    else:
                                                                        elements0 = None
                                                                        self._offset = index1
                                                                else:
                                                                    elements0 = None
                                                                    self._offset = index1
                                                            else:
                                                                elements0 = None
                                                                self._offset = index1
                                                        else:
                                                            elements0 = None
                                                            self._offset = index1
                                                    else:
                                                        elements0 = None
                                                        self._offset = index1
                                                else:
                                                    elements0 = None
                                                    self._offset = index1
                                            else:
                                                elements0 = None
                                                self._offset = index1
                                        else:
                                            elements0 = None
                                            self._offset = index1
                                    else:
                                        elements0 = None
                                        self._offset = index1
                                else:
                                    elements0 = None
                                    self._offset = index1
                            else:
                                elements0 = None
                                self._offset = index1
                        else:
                            elements0 = None
                            self._offset = index1
                    else:
                        elements0 = None
                        self._offset = index1
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = self._actions.make_indicator(self._input, index1, self._offset, elements0)
            self._offset = self._offset
        self._cache['indicatordef'][index0] = (address0, self._offset)
        return address0

    def _read_integerdef(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['integerdef'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        chunk0 = None
        if self._offset < self._input_size:
            chunk0 = self._input[self._offset:self._offset + 7]
        if chunk0 == 'INTEGER':
            address1 = TreeNode(self._input[self._offset:self._offset + 7], self._offset)
            self._offset = self._offset + 7
        else:
            address1 = FAILURE
            if self._offset > self._failure:
                self._failure = self._offset
                self._expected = []
            if self._offset == self._failure:
                self._expected.append('"INTEGER"')
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            address2 = self._read_SP()
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                index2 = self._offset
                index3, elements1 = self._offset, []
                address4 = FAILURE
                address4 = self._read_identifier()
                if address4 is not FAILURE:
                    elements1.append(address4)
                    address5 = FAILURE
                    address5 = self._read_EQUALS()
                    if address5 is not FAILURE:
                        elements1.append(address5)
                    else:
                        elements1 = None
                        self._offset = index3
                else:
                    elements1 = None
                    self._offset = index3
                if elements1 is None:
                    address3 = FAILURE
                else:
                    address3 = TreeNode17(self._input[index3:self._offset], index3, elements1)
                    self._offset = self._offset
                if address3 is FAILURE:
                    address3 = TreeNode(self._input[index2:index2], index2)
                    self._offset = index2
                if address3 is not FAILURE:
                    elements0.append(address3)
                    address6 = FAILURE
                    address6 = self._read_identifier()
                    if address6 is not FAILURE:
                        elements0.append(address6)
                        address7 = FAILURE
                        address7 = self._read_COMMA()
                        if address7 is not FAILURE:
                            elements0.append(address7)
                            address8 = FAILURE
                            address8 = self._read_SIZE()
                            if address8 is not FAILURE:
                                elements0.append(address8)
                                address9 = FAILURE
                                address9 = self._read_COMMA()
                                if address9 is not FAILURE:
                                    elements0.append(address9)
                                    address10 = FAILURE
                                    address10 = self._read_color()
                                    if address10 is not FAILURE:
                                        elements0.append(address10)
                                        address11 = FAILURE
                                        address11 = self._read_COMMA()
                                        if address11 is not FAILURE:
                                            elements0.append(address11)
                                            address12 = FAILURE
                                            address12 = self._read_color()
                                            if address12 is not FAILURE:
                                                elements0.append(address12)
                                                address13 = FAILURE
                                                address13 = self._read_COMMA()
                                                if address13 is not FAILURE:
                                                    elements0.append(address13)
                                                    address14 = FAILURE
                                                    address14 = self._read_string()
                                                    if address14 is not FAILURE:
                                                        elements0.append(address14)
                                                        address15 = FAILURE
                                                        address15 = self._read_COMMA()
                                                        if address15 is not FAILURE:
                                                            elements0.append(address15)
                                                            address16 = FAILURE
                                                            address16 = self._read_FUNC()
                                                            if address16 is not FAILURE:
                                                                elements0.append(address16)
                                                                address17 = FAILURE
                                                                address17 = self._read_SP()
                                                                if address17 is not FAILURE:
                                                                    elements0.append(address17)
                                                                    address18 = FAILURE
                                                                    chunk1 = None
                                                                    if self._offset < self._input_size:
                                                                        chunk1 = self._input[self._offset:self._offset + 1]
                                                                    if chunk1 == ';':
                                                                        address18 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                                                        self._offset = self._offset + 1
                                                                    else:
                                                                        address18 = FAILURE
                                                                        if self._offset > self._failure:
                                                                            self._failure = self._offset
                                                                            self._expected = []
                                                                        if self._offset == self._failure:
                                                                            self._expected.append('";"')
                                                                    if address18 is not FAILURE:
                                                                        elements0.append(address18)
                                                                    else:
                                                                        elements0 = None
                                                                        self._offset = index1
                                                                else:
                                                                    elements0 = None
                                                                    self._offset = index1
                                                            else:
                                                                elements0 = None
                                                                self._offset = index1
                                                        else:
                                                            elements0 = None
                                                            self._offset = index1
                                                    else:
                                                        elements0 = None
                                                        self._offset = index1
                                                else:
                                                    elements0 = None
                                                    self._offset = index1
                                            else:
                                                elements0 = None
                                                self._offset = index1
                                        else:
                                            elements0 = None
                                            self._offset = index1
                                    else:
                                        elements0 = None
                                        self._offset = index1
                                else:
                                    elements0 = None
                                    self._offset = index1
                            else:
                                elements0 = None
                                self._offset = index1
                        else:
                            elements0 = None
                            self._offset = index1
                    else:
                        elements0 = None
                        self._offset = index1
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = self._actions.make_integer(self._input, index1, self._offset, elements0)
            self._offset = self._offset
        self._cache['integerdef'][index0] = (address0, self._offset)
        return address0

    def _read_windowdef(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['windowdef'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        chunk0 = None
        if self._offset < self._input_size:
            chunk0 = self._input[self._offset:self._offset + 6]
        if chunk0 == 'WINDOW':
            address1 = TreeNode(self._input[self._offset:self._offset + 6], self._offset)
            self._offset = self._offset + 6
        else:
            address1 = FAILURE
            if self._offset > self._failure:
                self._failure = self._offset
                self._expected = []
            if self._offset == self._failure:
                self._expected.append('"WINDOW"')
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            address2 = self._read_SP()
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                index2 = self._offset
                index3, elements1 = self._offset, []
                address4 = FAILURE
                address4 = self._read_identifier()
                if address4 is not FAILURE:
                    elements1.append(address4)
                    address5 = FAILURE
                    address5 = self._read_EQUALS()
                    if address5 is not FAILURE:
                        elements1.append(address5)
                    else:
                        elements1 = None
                        self._offset = index3
                else:
                    elements1 = None
                    self._offset = index3
                if elements1 is None:
                    address3 = FAILURE
                else:
                    address3 = TreeNode19(self._input[index3:self._offset], index3, elements1)
                    self._offset = self._offset
                if address3 is FAILURE:
                    address3 = TreeNode(self._input[index2:index2], index2)
                    self._offset = index2
                if address3 is not FAILURE:
                    elements0.append(address3)
                    address6 = FAILURE
                    address6 = self._read_integer()
                    if address6 is not FAILURE:
                        elements0.append(address6)
                        address7 = FAILURE
                        chunk1 = None
                        if self._offset < self._input_size:
                            chunk1 = self._input[self._offset:self._offset + 1]
                        if chunk1 == 'x':
                            address7 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                            self._offset = self._offset + 1
                        else:
                            address7 = FAILURE
                            if self._offset > self._failure:
                                self._failure = self._offset
                                self._expected = []
                            if self._offset == self._failure:
                                self._expected.append('"x"')
                        if address7 is not FAILURE:
                            elements0.append(address7)
                            address8 = FAILURE
                            address8 = self._read_integer()
                            if address8 is not FAILURE:
                                elements0.append(address8)
                                address9 = FAILURE
                                address9 = self._read_COMMA()
                                if address9 is not FAILURE:
                                    elements0.append(address9)
                                    address10 = FAILURE
                                    address10 = self._read_color()
                                    if address10 is not FAILURE:
                                        elements0.append(address10)
                                        address11 = FAILURE
                                        address11 = self._read_COMMA()
                                        if address11 is not FAILURE:
                                            elements0.append(address11)
                                            address12 = FAILURE
                                            address12 = self._read_color()
                                            if address12 is not FAILURE:
                                                elements0.append(address12)
                                                address13 = FAILURE
                                                chunk2 = None
                                                if self._offset < self._input_size:
                                                    chunk2 = self._input[self._offset:self._offset + 1]
                                                if chunk2 == '\n':
                                                    address13 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                                    self._offset = self._offset + 1
                                                else:
                                                    address13 = FAILURE
                                                    if self._offset > self._failure:
                                                        self._failure = self._offset
                                                        self._expected = []
                                                    if self._offset == self._failure:
                                                        self._expected.append('"\\n"')
                                                if address13 is not FAILURE:
                                                    elements0.append(address13)
                                                    address14 = FAILURE
                                                    remaining0, index4, elements2, address15 = 0, self._offset, [], True
                                                    while address15 is not FAILURE:
                                                        address15 = self._read_windowitem()
                                                        if address15 is not FAILURE:
                                                            elements2.append(address15)
                                                            remaining0 -= 1
                                                    if remaining0 <= 0:
                                                        address14 = TreeNode(self._input[index4:self._offset], index4, elements2)
                                                        self._offset = self._offset
                                                    else:
                                                        address14 = FAILURE
                                                    if address14 is not FAILURE:
                                                        elements0.append(address14)
                                                        address16 = FAILURE
                                                        address16 = self._read_SP()
                                                        if address16 is not FAILURE:
                                                            elements0.append(address16)
                                                            address17 = FAILURE
                                                            chunk3 = None
                                                            if self._offset < self._input_size:
                                                                chunk3 = self._input[self._offset:self._offset + 1]
                                                            if chunk3 == ';':
                                                                address17 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                                                self._offset = self._offset + 1
                                                            else:
                                                                address17 = FAILURE
                                                                if self._offset > self._failure:
                                                                    self._failure = self._offset
                                                                    self._expected = []
                                                                if self._offset == self._failure:
                                                                    self._expected.append('";"')
                                                            if address17 is not FAILURE:
                                                                elements0.append(address17)
                                                            else:
                                                                elements0 = None
                                                                self._offset = index1
                                                        else:
                                                            elements0 = None
                                                            self._offset = index1
                                                    else:
                                                        elements0 = None
                                                        self._offset = index1
                                                else:
                                                    elements0 = None
                                                    self._offset = index1
                                            else:
                                                elements0 = None
                                                self._offset = index1
                                        else:
                                            elements0 = None
                                            self._offset = index1
                                    else:
                                        elements0 = None
                                        self._offset = index1
                                else:
                                    elements0 = None
                                    self._offset = index1
                            else:
                                elements0 = None
                                self._offset = index1
                        else:
                            elements0 = None
                            self._offset = index1
                    else:
                        elements0 = None
                        self._offset = index1
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = self._actions.make_window(self._input, index1, self._offset, elements0)
            self._offset = self._offset
        self._cache['windowdef'][index0] = (address0, self._offset)
        return address0

    def _read_windowitem(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['windowitem'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        address1 = self._read_AT()
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            address2 = self._read_integer()
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                address3 = self._read_COMMA()
                if address3 is not FAILURE:
                    elements0.append(address3)
                    address4 = FAILURE
                    address4 = self._read_integer()
                    if address4 is not FAILURE:
                        elements0.append(address4)
                        address5 = FAILURE
                        address5 = self._read_COLON()
                        if address5 is not FAILURE:
                            elements0.append(address5)
                            address6 = FAILURE
                            index2 = self._offset
                            index3, elements1 = self._offset, []
                            address7 = FAILURE
                            address7 = self._read_SP()
                            if address7 is not FAILURE:
                                elements1.append(address7)
                                address8 = FAILURE
                                address8 = self._read_definition()
                                if address8 is not FAILURE:
                                    elements1.append(address8)
                                else:
                                    elements1 = None
                                    self._offset = index3
                            else:
                                elements1 = None
                                self._offset = index3
                            if elements1 is None:
                                address6 = FAILURE
                            else:
                                address6 = TreeNode21(self._input[index3:self._offset], index3, elements1)
                                self._offset = self._offset
                            if address6 is FAILURE:
                                self._offset = index2
                                index4, elements2 = self._offset, []
                                address9 = FAILURE
                                address9 = self._read_identifier()
                                if address9 is not FAILURE:
                                    elements2.append(address9)
                                    address10 = FAILURE
                                    address10 = self._read_SP()
                                    if address10 is not FAILURE:
                                        elements2.append(address10)
                                        address11 = FAILURE
                                        chunk0 = None
                                        if self._offset < self._input_size:
                                            chunk0 = self._input[self._offset:self._offset + 1]
                                        if chunk0 == ';':
                                            address11 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                            self._offset = self._offset + 1
                                        else:
                                            address11 = FAILURE
                                            if self._offset > self._failure:
                                                self._failure = self._offset
                                                self._expected = []
                                            if self._offset == self._failure:
                                                self._expected.append('";"')
                                        if address11 is not FAILURE:
                                            elements2.append(address11)
                                        else:
                                            elements2 = None
                                            self._offset = index4
                                    else:
                                        elements2 = None
                                        self._offset = index4
                                else:
                                    elements2 = None
                                    self._offset = index4
                                if elements2 is None:
                                    address6 = FAILURE
                                else:
                                    address6 = TreeNode22(self._input[index4:self._offset], index4, elements2)
                                    self._offset = self._offset
                                if address6 is FAILURE:
                                    self._offset = index2
                            if address6 is not FAILURE:
                                elements0.append(address6)
                                address12 = FAILURE
                                chunk1 = None
                                if self._offset < self._input_size:
                                    chunk1 = self._input[self._offset:self._offset + 1]
                                if chunk1 == '\n':
                                    address12 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                                    self._offset = self._offset + 1
                                else:
                                    address12 = FAILURE
                                    if self._offset > self._failure:
                                        self._failure = self._offset
                                        self._expected = []
                                    if self._offset == self._failure:
                                        self._expected.append('"\\n"')
                                if address12 is not FAILURE:
                                    elements0.append(address12)
                                else:
                                    elements0 = None
                                    self._offset = index1
                            else:
                                elements0 = None
                                self._offset = index1
                        else:
                            elements0 = None
                            self._offset = index1
                    else:
                        elements0 = None
                        self._offset = index1
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = TreeNode20(self._input[index1:self._offset], index1, elements0)
            self._offset = self._offset
        self._cache['windowitem'][index0] = (address0, self._offset)
        return address0

    def _read_integer(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['integer'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        remaining0, index1, elements0, address1 = 1, self._offset, [], True
        while address1 is not FAILURE:
            chunk0 = None
            if self._offset < self._input_size:
                chunk0 = self._input[self._offset:self._offset + 1]
            if chunk0 is not None and Grammar.REGEX_1.search(chunk0):
                address1 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                self._offset = self._offset + 1
            else:
                address1 = FAILURE
                if self._offset > self._failure:
                    self._failure = self._offset
                    self._expected = []
                if self._offset == self._failure:
                    self._expected.append('[0-9]')
            if address1 is not FAILURE:
                elements0.append(address1)
                remaining0 -= 1
        if remaining0 <= 0:
            address0 = TreeNode(self._input[index1:self._offset], index1, elements0)
            self._offset = self._offset
        else:
            address0 = FAILURE
        self._cache['integer'][index0] = (address0, self._offset)
        return address0

    def _read_identifier(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['identifier'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        chunk0 = None
        if self._offset < self._input_size:
            chunk0 = self._input[self._offset:self._offset + 1]
        if chunk0 is not None and Grammar.REGEX_2.search(chunk0):
            address1 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
            self._offset = self._offset + 1
        else:
            address1 = FAILURE
            if self._offset > self._failure:
                self._failure = self._offset
                self._expected = []
            if self._offset == self._failure:
                self._expected.append('[a-zA-Z_]')
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            remaining0, index2, elements1, address3 = 0, self._offset, [], True
            while address3 is not FAILURE:
                chunk1 = None
                if self._offset < self._input_size:
                    chunk1 = self._input[self._offset:self._offset + 1]
                if chunk1 is not None and Grammar.REGEX_3.search(chunk1):
                    address3 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                    self._offset = self._offset + 1
                else:
                    address3 = FAILURE
                    if self._offset > self._failure:
                        self._failure = self._offset
                        self._expected = []
                    if self._offset == self._failure:
                        self._expected.append('[a-zA-Z0-9_.]')
                if address3 is not FAILURE:
                    elements1.append(address3)
                    remaining0 -= 1
            if remaining0 <= 0:
                address2 = TreeNode(self._input[index2:self._offset], index2, elements1)
                self._offset = self._offset
            else:
                address2 = FAILURE
            if address2 is not FAILURE:
                elements0.append(address2)
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = TreeNode(self._input[index1:self._offset], index1, elements0)
            self._offset = self._offset
        self._cache['identifier'][index0] = (address0, self._offset)
        return address0

    def _read_cidentifier(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['cidentifier'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        remaining0, index1, elements0, address1 = 1, self._offset, [], True
        while address1 is not FAILURE:
            index2, elements1 = self._offset, []
            address2 = FAILURE
            index3 = self._offset
            chunk0 = None
            if self._offset < self._input_size:
                chunk0 = self._input[self._offset:self._offset + 2]
            if chunk0 == '::':
                address2 = TreeNode(self._input[self._offset:self._offset + 2], self._offset)
                self._offset = self._offset + 2
            else:
                address2 = FAILURE
                if self._offset > self._failure:
                    self._failure = self._offset
                    self._expected = []
                if self._offset == self._failure:
                    self._expected.append('"::"')
            if address2 is FAILURE:
                address2 = TreeNode(self._input[index3:index3], index3)
                self._offset = index3
            if address2 is not FAILURE:
                elements1.append(address2)
                address3 = FAILURE
                address3 = self._read_identifier()
                if address3 is not FAILURE:
                    elements1.append(address3)
                else:
                    elements1 = None
                    self._offset = index2
            else:
                elements1 = None
                self._offset = index2
            if elements1 is None:
                address1 = FAILURE
            else:
                address1 = TreeNode23(self._input[index2:self._offset], index2, elements1)
                self._offset = self._offset
            if address1 is not FAILURE:
                elements0.append(address1)
                remaining0 -= 1
        if remaining0 <= 0:
            address0 = TreeNode(self._input[index1:self._offset], index1, elements0)
            self._offset = self._offset
        else:
            address0 = FAILURE
        self._cache['cidentifier'][index0] = (address0, self._offset)
        return address0

    def _read_string(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['string'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        chunk0 = None
        if self._offset < self._input_size:
            chunk0 = self._input[self._offset:self._offset + 1]
        if chunk0 == '"':
            address1 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
            self._offset = self._offset + 1
        else:
            address1 = FAILURE
            if self._offset > self._failure:
                self._failure = self._offset
                self._expected = []
            if self._offset == self._failure:
                self._expected.append('"\\""')
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            remaining0, index2, elements1, address3 = 0, self._offset, [], True
            while address3 is not FAILURE:
                chunk1 = None
                if self._offset < self._input_size:
                    chunk1 = self._input[self._offset:self._offset + 1]
                if chunk1 is not None and Grammar.REGEX_4.search(chunk1):
                    address3 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                    self._offset = self._offset + 1
                else:
                    address3 = FAILURE
                    if self._offset > self._failure:
                        self._failure = self._offset
                        self._expected = []
                    if self._offset == self._failure:
                        self._expected.append('[^"]')
                if address3 is not FAILURE:
                    elements1.append(address3)
                    remaining0 -= 1
            if remaining0 <= 0:
                address2 = TreeNode(self._input[index2:self._offset], index2, elements1)
                self._offset = self._offset
            else:
                address2 = FAILURE
            if address2 is not FAILURE:
                elements0.append(address2)
                address4 = FAILURE
                chunk2 = None
                if self._offset < self._input_size:
                    chunk2 = self._input[self._offset:self._offset + 1]
                if chunk2 == '"':
                    address4 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                    self._offset = self._offset + 1
                else:
                    address4 = FAILURE
                    if self._offset > self._failure:
                        self._failure = self._offset
                        self._expected = []
                    if self._offset == self._failure:
                        self._expected.append('"\\""')
                if address4 is not FAILURE:
                    elements0.append(address4)
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = TreeNode(self._input[index1:self._offset], index1, elements0)
            self._offset = self._offset
        self._cache['string'][index0] = (address0, self._offset)
        return address0

    def _read_COMMA(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['COMMA'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        address1 = self._read_SP()
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            chunk0 = None
            if self._offset < self._input_size:
                chunk0 = self._input[self._offset:self._offset + 1]
            if chunk0 == ',':
                address2 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                self._offset = self._offset + 1
            else:
                address2 = FAILURE
                if self._offset > self._failure:
                    self._failure = self._offset
                    self._expected = []
                if self._offset == self._failure:
                    self._expected.append('","')
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                address3 = self._read_SP()
                if address3 is not FAILURE:
                    elements0.append(address3)
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = TreeNode24(self._input[index1:self._offset], index1, elements0)
            self._offset = self._offset
        self._cache['COMMA'][index0] = (address0, self._offset)
        return address0

    def _read_COLON(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['COLON'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        address1 = self._read_SP()
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            chunk0 = None
            if self._offset < self._input_size:
                chunk0 = self._input[self._offset:self._offset + 1]
            if chunk0 == ':':
                address2 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                self._offset = self._offset + 1
            else:
                address2 = FAILURE
                if self._offset > self._failure:
                    self._failure = self._offset
                    self._expected = []
                if self._offset == self._failure:
                    self._expected.append('":"')
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                address3 = self._read_SP()
                if address3 is not FAILURE:
                    elements0.append(address3)
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = TreeNode25(self._input[index1:self._offset], index1, elements0)
            self._offset = self._offset
        self._cache['COLON'][index0] = (address0, self._offset)
        return address0

    def _read_EQUALS(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['EQUALS'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        address1 = self._read_SP()
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            chunk0 = None
            if self._offset < self._input_size:
                chunk0 = self._input[self._offset:self._offset + 1]
            if chunk0 == '=':
                address2 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                self._offset = self._offset + 1
            else:
                address2 = FAILURE
                if self._offset > self._failure:
                    self._failure = self._offset
                    self._expected = []
                if self._offset == self._failure:
                    self._expected.append('"="')
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                address3 = self._read_SP()
                if address3 is not FAILURE:
                    elements0.append(address3)
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = TreeNode26(self._input[index1:self._offset], index1, elements0)
            self._offset = self._offset
        self._cache['EQUALS'][index0] = (address0, self._offset)
        return address0

    def _read_AT(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['AT'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        address1 = self._read_SP()
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            chunk0 = None
            if self._offset < self._input_size:
                chunk0 = self._input[self._offset:self._offset + 1]
            if chunk0 == '@':
                address2 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                self._offset = self._offset + 1
            else:
                address2 = FAILURE
                if self._offset > self._failure:
                    self._failure = self._offset
                    self._expected = []
                if self._offset == self._failure:
                    self._expected.append('"@"')
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                address3 = self._read_SP()
                if address3 is not FAILURE:
                    elements0.append(address3)
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = TreeNode27(self._input[index1:self._offset], index1, elements0)
            self._offset = self._offset
        self._cache['AT'][index0] = (address0, self._offset)
        return address0

    def _read_SP(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['SP'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        remaining0, index1, elements0, address1 = 0, self._offset, [], True
        while address1 is not FAILURE:
            chunk0 = None
            if self._offset < self._input_size:
                chunk0 = self._input[self._offset:self._offset + 1]
            if chunk0 == ' ':
                address1 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                self._offset = self._offset + 1
            else:
                address1 = FAILURE
                if self._offset > self._failure:
                    self._failure = self._offset
                    self._expected = []
                if self._offset == self._failure:
                    self._expected.append('" "')
            if address1 is not FAILURE:
                elements0.append(address1)
                remaining0 -= 1
        if remaining0 <= 0:
            address0 = TreeNode(self._input[index1:self._offset], index1, elements0)
            self._offset = self._offset
        else:
            address0 = FAILURE
        self._cache['SP'][index0] = (address0, self._offset)
        return address0

    def _read_comment(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['comment'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        chunk0 = None
        if self._offset < self._input_size:
            chunk0 = self._input[self._offset:self._offset + 1]
        if chunk0 == '#':
            address1 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
            self._offset = self._offset + 1
        else:
            address1 = FAILURE
            if self._offset > self._failure:
                self._failure = self._offset
                self._expected = []
            if self._offset == self._failure:
                self._expected.append('"#"')
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            remaining0, index2, elements1, address3 = 0, self._offset, [], True
            while address3 is not FAILURE:
                chunk1 = None
                if self._offset < self._input_size:
                    chunk1 = self._input[self._offset:self._offset + 1]
                if chunk1 is not None and Grammar.REGEX_5.search(chunk1):
                    address3 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                    self._offset = self._offset + 1
                else:
                    address3 = FAILURE
                    if self._offset > self._failure:
                        self._failure = self._offset
                        self._expected = []
                    if self._offset == self._failure:
                        self._expected.append('[^\\n]')
                if address3 is not FAILURE:
                    elements1.append(address3)
                    remaining0 -= 1
            if remaining0 <= 0:
                address2 = TreeNode(self._input[index2:self._offset], index2, elements1)
                self._offset = self._offset
            else:
                address2 = FAILURE
            if address2 is not FAILURE:
                elements0.append(address2)
                address4 = FAILURE
                chunk2 = None
                if self._offset < self._input_size:
                    chunk2 = self._input[self._offset:self._offset + 1]
                if chunk2 == '\n':
                    address4 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                    self._offset = self._offset + 1
                else:
                    address4 = FAILURE
                    if self._offset > self._failure:
                        self._failure = self._offset
                        self._expected = []
                    if self._offset == self._failure:
                        self._expected.append('"\\n"')
                if address4 is not FAILURE:
                    elements0.append(address4)
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = TreeNode(self._input[index1:self._offset], index1, elements0)
            self._offset = self._offset
        self._cache['comment'][index0] = (address0, self._offset)
        return address0

    def _read_rgb(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['rgb'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        address1 = self._read_integer()
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            chunk0 = None
            if self._offset < self._input_size:
                chunk0 = self._input[self._offset:self._offset + 1]
            if chunk0 == '/':
                address2 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                self._offset = self._offset + 1
            else:
                address2 = FAILURE
                if self._offset > self._failure:
                    self._failure = self._offset
                    self._expected = []
                if self._offset == self._failure:
                    self._expected.append('"/"')
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                address3 = self._read_integer()
                if address3 is not FAILURE:
                    elements0.append(address3)
                    address4 = FAILURE
                    chunk1 = None
                    if self._offset < self._input_size:
                        chunk1 = self._input[self._offset:self._offset + 1]
                    if chunk1 == '/':
                        address4 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                        self._offset = self._offset + 1
                    else:
                        address4 = FAILURE
                        if self._offset > self._failure:
                            self._failure = self._offset
                            self._expected = []
                        if self._offset == self._failure:
                            self._expected.append('"/"')
                    if address4 is not FAILURE:
                        elements0.append(address4)
                        address5 = FAILURE
                        address5 = self._read_integer()
                        if address5 is not FAILURE:
                            elements0.append(address5)
                        else:
                            elements0 = None
                            self._offset = index1
                    else:
                        elements0 = None
                        self._offset = index1
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = TreeNode28(self._input[index1:self._offset], index1, elements0)
            self._offset = self._offset
        self._cache['rgb'][index0] = (address0, self._offset)
        return address0

    def _read_SIZE(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['SIZE'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1, elements0 = self._offset, []
        address1 = FAILURE
        address1 = self._read_integer()
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            chunk0 = None
            if self._offset < self._input_size:
                chunk0 = self._input[self._offset:self._offset + 1]
            if chunk0 == 'x':
                address2 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                self._offset = self._offset + 1
            else:
                address2 = FAILURE
                if self._offset > self._failure:
                    self._failure = self._offset
                    self._expected = []
                if self._offset == self._failure:
                    self._expected.append('"x"')
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                address3 = self._read_integer()
                if address3 is not FAILURE:
                    elements0.append(address3)
                else:
                    elements0 = None
                    self._offset = index1
            else:
                elements0 = None
                self._offset = index1
        else:
            elements0 = None
            self._offset = index1
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = TreeNode29(self._input[index1:self._offset], index1, elements0)
            self._offset = self._offset
        self._cache['SIZE'][index0] = (address0, self._offset)
        return address0

    def _read_FUNC(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['FUNC'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1 = self._offset
        index2, elements0 = self._offset, []
        address1 = FAILURE
        address1 = self._read_cidentifier()
        if address1 is not FAILURE:
            elements0.append(address1)
            address2 = FAILURE
            chunk0 = None
            if self._offset < self._input_size:
                chunk0 = self._input[self._offset:self._offset + 1]
            if chunk0 == '(':
                address2 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                self._offset = self._offset + 1
            else:
                address2 = FAILURE
                if self._offset > self._failure:
                    self._failure = self._offset
                    self._expected = []
                if self._offset == self._failure:
                    self._expected.append('"("')
            if address2 is not FAILURE:
                elements0.append(address2)
                address3 = FAILURE
                index3 = self._offset
                address3 = self._read_integer()
                if address3 is FAILURE:
                    self._offset = index3
                    address3 = self._read_identifier()
                    if address3 is FAILURE:
                        self._offset = index3
                        address3 = self._read_SP()
                        if address3 is FAILURE:
                            self._offset = index3
                if address3 is not FAILURE:
                    elements0.append(address3)
                    address4 = FAILURE
                    chunk1 = None
                    if self._offset < self._input_size:
                        chunk1 = self._input[self._offset:self._offset + 1]
                    if chunk1 == ')':
                        address4 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                        self._offset = self._offset + 1
                    else:
                        address4 = FAILURE
                        if self._offset > self._failure:
                            self._failure = self._offset
                            self._expected = []
                        if self._offset == self._failure:
                            self._expected.append('")"')
                    if address4 is not FAILURE:
                        elements0.append(address4)
                    else:
                        elements0 = None
                        self._offset = index2
                else:
                    elements0 = None
                    self._offset = index2
            else:
                elements0 = None
                self._offset = index2
        else:
            elements0 = None
            self._offset = index2
        if elements0 is None:
            address0 = FAILURE
        else:
            address0 = TreeNode30(self._input[index2:self._offset], index2, elements0)
            self._offset = self._offset
        if address0 is FAILURE:
            self._offset = index1
            chunk2 = None
            if self._offset < self._input_size:
                chunk2 = self._input[self._offset:self._offset + 1]
            if chunk2 == '*':
                address0 = TreeNode(self._input[self._offset:self._offset + 1], self._offset)
                self._offset = self._offset + 1
            else:
                address0 = FAILURE
                if self._offset > self._failure:
                    self._failure = self._offset
                    self._expected = []
                if self._offset == self._failure:
                    self._expected.append('"*"')
            if address0 is FAILURE:
                self._offset = index1
        self._cache['FUNC'][index0] = (address0, self._offset)
        return address0

    def _read_const(self):
        address0, index0 = FAILURE, self._offset
        cached = self._cache['const'].get(index0)
        if cached:
            self._offset = cached[1]
            return cached[0]
        index1 = self._offset
        address0 = self._read_integer()
        if address0 is FAILURE:
            self._offset = index1
            address0 = self._read_identifier()
            if address0 is FAILURE:
                self._offset = index1
        self._cache['const'][index0] = (address0, self._offset)
        return address0


class Parser(Grammar):
    def __init__(self, input, actions, types):
        self._input = input
        self._input_size = len(input)
        self._actions = actions
        self._types = types
        self._offset = 0
        self._cache = defaultdict(dict)
        self._failure = 0
        self._expected = []

    def parse(self):
        tree = self._read_input()
        if tree is not FAILURE and self._offset == self._input_size:
            return tree
        if not self._expected:
            self._failure = self._offset
            self._expected.append('<EOF>')
        raise ParseError(format_error(self._input, self._failure, self._expected))


def format_error(input, offset, expected):
    lines, line_no, position = input.split('\n'), 0, 0
    while position <= offset:
        position += len(lines[line_no]) + 1
        line_no += 1
    message, line = 'Line ' + str(line_no) + ': expected ' + ', '.join(expected) + '\n', lines[line_no - 1]
    message += line + '\n'
    position -= len(line) + 1
    message += ' ' * (offset - position)
    return message + '^'

def parse(input, actions=None, types=None):
    parser = Parser(input, actions, types)
    return parser.parse()
