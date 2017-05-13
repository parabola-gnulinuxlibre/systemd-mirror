#  -*- Mode: python; coding: utf-8; indent-tabs-mode: nil -*- */
#
#  This file is part of systemd.
#
#  Copyright 2012-2013 Zbigniew Jędrzejewski-Szmek
#
#  systemd is free software; you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation; either version 2.1 of the License, or
#  (at your option) any later version.
#
#  systemd is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with systemd; If not, see <http://www.gnu.org/licenses/>.

from lxml import etree as tree
import os.path

shared = { 'custom-entities.ent',
           'less-variables.xml',
           'standard-conf.xml',
           'standard-options.xml',
           'user-system-options.xml' }

class CustomResolver(tree.Resolver):
    def resolve(self, url, id, context):
        basename = os.path.basename(url)
        if not basename in shared:
            return None

        topsrcdir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

        makename = os.path.join('$(topsrcdir)', 'man', basename)
        lxmlname = os.path.join(topsrcdir, 'man', basename)
        if basename == 'custom-entities.ent':
            makename = os.path.join('$(topoutdir)', 'man', basename)
            lxmlname += '.in'

        _deps.add(makename)
        return self.resolve_filename(lxmlname, context)

_parser = tree.XMLParser()
_parser.resolvers.add(CustomResolver())
_deps = set()
def xml_parse(page):
    _deps.clear()
    doc = tree.parse(page, _parser)
    doc.xinclude()
    return doc, _deps.copy()
def xml_print(xml):
    return tree.tostring(xml, pretty_print=True, encoding='utf-8')
