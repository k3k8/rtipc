#!/bin/sh
#############################################################################
#
#  Copyright 2024 Bjarne von Horn <vh@igh.de>
#
#  This file is part of the rtipc library.
#
#  The rtipc library is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  The rtipc library is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
#  License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with the rtipc library. If not, see <http://www.gnu.org/licenses/>.
#
#############################################################################

set -e

readelf --dyn-syms --wide "$1" | \
    awk '$4 == "FUNC" && $5 != "WEAK" && $7 != "UND" && $8 !~ /@LIBRTIPC/ { print "Unversioned symbol: " $8; err = 1; } END { exit err; }'
