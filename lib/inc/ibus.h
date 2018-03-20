#ifndef __IBUS_H
#define __IBUS_H

#include <stm8s.h>
#include <stdio.h>
#include <yfuns.h>
#include <stdlib.h>

void init_ibus(u8 number_of_channels);

void read_channel_data(u8 * data, u8 length);

void write_channel_data(u8 * data, u8 length);

#endif