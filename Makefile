##
## This file is part of the libopencm3 project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
## Copyright (C) 2015 Chuck McManis <cmcmanis@mcmanis.com>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FTNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more detail.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##


SRCDIR   = ./


SOURCE    =  motion_control.c gcode.c spindle_control.c coolant_control.c \
             protocol.c stepper.c eeprom.c settings.c planner.c nuts_bolts.c limits.c \
             print.c probe.c report.c system.c stm_main.c serial_stm32.c delay.c hw_control.c

OBJS = $(addprefix $(BUILDDIR),$(addprefix $(SRCDIR),$(SOURCE:.c=.o)))


$(info OBJS $(OBJS))    

BINARY = stm_main

OPENCM3_DIR=../libopencm3

LDSCRIPT = nucleo-f411re.ld

include Makefile.include

