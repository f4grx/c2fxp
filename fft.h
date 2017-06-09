/*
 * fft - fixed point Fast Fourier Transform
 * Copyright (C) 2017  Sebastien F4GRX <f4grx@f4grx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __FFT__H__
#define __FFT__H__

#include "fxpmath.h"

int q15_fft(q15_t *datar, q15_t *datai, uint32_t n);

#endif /* __FFT__H__ */

