############################################################################
# 
# Copyright 2021 Lee Mitchell <lee@indigopepper.com>
# This file is part of ADF435xCFG
# 
# ADF435xCFG is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
# 
# ADF435xCFG is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software.  If not,
# see <http://www.gnu.org/licenses/>.
# 
############################################################################

TARGET=adf435xcfg.exe

CC=gcc

all:
ifeq ($(OS),Windows_NT)
	$(CC) -o $(TARGET) main.c ch341.c adf435x.c -L . -lusb-1.0
else
	$(CC) -o $(TARGET) main.c
endif

clean:
	rm -rf $(TARGET)