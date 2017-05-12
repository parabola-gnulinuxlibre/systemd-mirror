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

class CustomResolver(tree.Resolver):
    def resolve(self, url, id, context):
        if ':' in url:
            return None
        if not os.path.exists(url):
            srcdir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
            basename = os.path.basename(url)
            if basename == 'custom-entities.ent':
                basename = 'custom-entities.ent.in'
            url = os.path.join(srcdir, 'man', basename)
        return self.resolve_filename(url, context)

_parser = tree.XMLParser()
_parser.resolvers.add(CustomResolver())
def xml_parse(page):
    doc = tree.parse(page, _parser)
    doc.xinclude()
    return doc
def xml_print(xml):
    return tree.tostring(xml, pretty_print=True, encoding='utf-8')
