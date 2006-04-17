/*	$OpenBSD: src/sys/dev/i2c/adt7460.c,v 1.13 2006/04/17 06:20:00 deraadt Exp $	*/

/*
 * Copyright (c) 2005 Mark Kettenis
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/sensors.h>

#include <dev/i2c/i2cvar.h>

/* ADT7460 registers */
#define ADT7460_2_5V		0x20
#define ADT7460_VCCP		0x21
#define ADT7460_VCC		0x22
#define ADT7460_V5		0x23
#define ADT7460_V12		0x24
#define ADT7460_REM1_TEMP	0x25
#define ADT7460_LOCAL_TEMP	0x26
#define ADT7460_REM2_TEMP	0x27
#define ADT7460_TACH1L		0x28
#define ADT7460_TACH1H		0x29
#define ADT7460_TACH2L		0x2a
#define ADT7460_TACH2H		0x2b
#define ADT7460_TACH3L		0x2c
#define ADT7460_TACH3H		0x2d
#define ADT7460_TACH4L		0x2e
#define ADT7460_TACH4H		0x2f
#define ADT7460_REVISION	0x3f
#define ADT7460_CONFIG		0x40
#define ADT7460_CONFIG_Vcc	0x80

/* Sensors */
#define ADT_2_5V		0
#define ADT_VCCP		1
#define ADT_VCC			2
#define ADT_V5			3
#define ADT_V12			4
#define ADT_REM1_TEMP		5
#define ADT_LOCAL_TEMP		6
#define ADT_REM2_TEMP		7
#define ADT_TACH1		8
#define ADT_TACH2		9
#define ADT_TACH3		10
#define ADT_TACH4		11
#define ADT_NUM_SENSORS		12

struct adt_chip {
	const char	*name;
	short		ratio[5];
	int		type;
	short		vcc;
} adt_chips[] = {
	/* register	0x20  0x21  0x22  0x23  0x24	type	*/
	/* 		2.5v  vccp   vcc    5v   12v		*/

	{ "adt7460",	{ 2500,    0, 3300,    0,     0 },	7460,	5000 },
	{ "adt7467",	{ 2500, 2250, 3300, 5000, 12000 },	7467,	5000 },
	{ "adt7476",	{ 2500, 2250, 3300, 5000, 12000 },	7476,	   0 },
	{ "adm1027",	{ 2500, 2250, 3300, 5000, 12000 },	1027,	5000 },
	{ "lm85",	{ 2500, 2250, 3300, 5000, 12000 },	7467,	   0 },
	{ "emc6d100",	{ 2500, 2250, 3300, 5000, 12000 },	6100,	   0 },
	{ "emc6w201",	{ 2500, 2250, 3300, 5000, 12000 },	6201,	   0 },
	{ "lm96000",	{ 2500, 2250, 3300, 5000, 12000 },	96000,	   0 },
	{ "sch5017",	{ 5000, 2250, 3300, 5000, 12000 },	5017,	   0 }
};

struct adt_softc {
	struct device sc_dev;
	i2c_tag_t sc_tag;
	i2c_addr_t sc_addr;
	u_int8_t sc_conf;
	struct adt_chip *chip;

	struct sensor sc_sensor[ADT_NUM_SENSORS];
};

int	adt_match(struct device *, void *, void *);
void	adt_attach(struct device *, struct device *, void *);

void	adt_refresh(void *);

struct cfattach adt_ca = {
	sizeof(struct adt_softc), adt_match, adt_attach
};

struct cfdriver adt_cd = {
	NULL, "adt", DV_DULL
};

int
adt_match(struct device *parent, void *match, void *aux)
{
	struct i2c_attach_args *ia = aux;
	int i;

	for (i = 0; i < sizeof(adt_chips) / sizeof(adt_chips[0]); i++)
		if (strcmp(ia->ia_name, adt_chips[i].name) == 0)
			return (1);
	return (0);
}

void
adt_attach(struct device *parent, struct device *self, void *aux)
{
	struct adt_softc *sc = (struct adt_softc *)self;
	struct i2c_attach_args *ia = aux;
	u_int8_t cmd, rev, data;
	int i;

	sc->sc_tag = ia->ia_tag;
	sc->sc_addr = ia->ia_addr;

	iic_acquire_bus(sc->sc_tag, 0);

	for (i = 0; i < sizeof(adt_chips) / sizeof(adt_chips[0]); i++) {
		if (strcmp(ia->ia_name, adt_chips[i].name) == 0) {
			sc->chip = &adt_chips[i];
			break;
		}
	}

	cmd = ADT7460_REVISION;
	if (iic_exec(sc->sc_tag, I2C_OP_READ_WITH_STOP,
	    sc->sc_addr, &cmd, sizeof cmd, &rev, sizeof rev, 0)) {
		iic_release_bus(sc->sc_tag, 0);
		printf(": cannot read REV register\n");
		return;
	}

	cmd = ADT7460_CONFIG;
	if (iic_exec(sc->sc_tag, I2C_OP_READ_WITH_STOP,
	    sc->sc_addr, &cmd, sizeof cmd, &sc->sc_conf, sizeof sc->sc_conf, 0)) {
		iic_release_bus(sc->sc_tag, 0);
		printf(": cannot read config register\n");
		return;
	}

	if (sc->chip->type == 7460) {
		data = 1;
		cmd = ADT7460_CONFIG;
		if (iic_exec(sc->sc_tag, I2C_OP_WRITE_WITH_STOP,
		    sc->sc_addr, &cmd, sizeof cmd, &data, sizeof data, 0)) {
			iic_release_bus(sc->sc_tag, 0);
			printf(": cannot set control register\n");
			return;
		}
	}

	iic_release_bus(sc->sc_tag, 0);

	printf(": %s rev 0x%02x", ia->ia_name, rev);

	/* Initialize sensor data. */
	for (i = 0; i < ADT_NUM_SENSORS; i++)
		strlcpy(sc->sc_sensor[i].device, sc->sc_dev.dv_xname,
		    sizeof(sc->sc_sensor[i].device));

	sc->sc_sensor[ADT_2_5V].type = SENSOR_VOLTS_DC;
	strlcpy(sc->sc_sensor[ADT_2_5V].desc, "+2.5Vin",
	    sizeof(sc->sc_sensor[ADT_2_5V].desc));
	if (sc->chip->type == 5017)
		strlcpy(sc->sc_sensor[ADT_2_5V].desc, "+5VTR",
		    sizeof(sc->sc_sensor[ADT_2_5V].desc));

	sc->sc_sensor[ADT_VCCP].type = SENSOR_VOLTS_DC;
	strlcpy(sc->sc_sensor[ADT_VCCP].desc, "Vccp",
	    sizeof(sc->sc_sensor[ADT_VCCP].desc));

	sc->sc_sensor[ADT_VCC].type = SENSOR_VOLTS_DC;
	strlcpy(sc->sc_sensor[ADT_VCC].desc, "Vcc",
	    sizeof(sc->sc_sensor[ADT_VCC].desc));

	sc->sc_sensor[ADT_V5].type = SENSOR_VOLTS_DC;
	strlcpy(sc->sc_sensor[ADT_V5].desc, "+5V",
	    sizeof(sc->sc_sensor[ADT_V5].desc));

	sc->sc_sensor[ADT_V12].type = SENSOR_VOLTS_DC;
	strlcpy(sc->sc_sensor[ADT_V12].desc, "+12V",
	    sizeof(sc->sc_sensor[ADT_V12].desc));

	sc->sc_sensor[ADT_REM1_TEMP].type = SENSOR_TEMP;
	strlcpy(sc->sc_sensor[ADT_REM1_TEMP].desc, "Remote1 Temp",
	    sizeof(sc->sc_sensor[ADT_REM1_TEMP].desc));

	sc->sc_sensor[ADT_LOCAL_TEMP].type = SENSOR_TEMP;
	strlcpy(sc->sc_sensor[ADT_LOCAL_TEMP].desc, "Internal Temp",
	    sizeof(sc->sc_sensor[ADT_LOCAL_TEMP].desc));

	sc->sc_sensor[ADT_REM2_TEMP].type = SENSOR_TEMP;
	strlcpy(sc->sc_sensor[ADT_REM2_TEMP].desc, "Remote2 Temp",
	    sizeof(sc->sc_sensor[ADT_REM2_TEMP].desc));

	sc->sc_sensor[ADT_TACH1].type = SENSOR_FANRPM;
	strlcpy(sc->sc_sensor[ADT_TACH1].desc, "TACH1",
	    sizeof(sc->sc_sensor[ADT_TACH1].desc));

	sc->sc_sensor[ADT_TACH2].type = SENSOR_FANRPM;
	strlcpy(sc->sc_sensor[ADT_TACH2].desc, "TACH2",
	    sizeof(sc->sc_sensor[ADT_TACH2].desc));

	sc->sc_sensor[ADT_TACH3].type = SENSOR_FANRPM;
	strlcpy(sc->sc_sensor[ADT_TACH3].desc, "TACH3",
	    sizeof(sc->sc_sensor[ADT_TACH3].desc));

	sc->sc_sensor[ADT_TACH4].type = SENSOR_FANRPM;
	strlcpy(sc->sc_sensor[ADT_TACH4].desc, "TACH4",
	    sizeof(sc->sc_sensor[ADT_TACH4].desc));

	if (sensor_task_register(sc, adt_refresh, 5)) {
		printf(", unable to register update task\n");
		return;
	}

	for (i = 0; i < ADT_NUM_SENSORS; i++) {
		sc->sc_sensor[i].flags &= ~SENSOR_FINVALID;
		sensor_add(&sc->sc_sensor[i]);
	}

	printf("\n");
}

struct {
	char		sensor;
	u_int8_t	cmd;
	u_char		index;
} worklist[] = {
	{ ADT_2_5V, ADT7460_2_5V, 0 },
	{ ADT_VCCP, ADT7460_VCCP, 1 },
	{ ADT_VCC, ADT7460_VCC, 2 },
	{ ADT_V5, ADT7460_V5, 3 },
	{ ADT_V12, ADT7460_V12, 4 },
	{ ADT_REM1_TEMP, ADT7460_REM1_TEMP },
	{ ADT_LOCAL_TEMP, ADT7460_LOCAL_TEMP },
	{ ADT_REM2_TEMP, ADT7460_REM2_TEMP },
	{ ADT_TACH1, ADT7460_TACH1L },
	{ ADT_TACH2, ADT7460_TACH2L },
	{ ADT_TACH3, ADT7460_TACH3L },
	{ ADT_TACH4, ADT7460_TACH4L },
};

void
adt_refresh(void *arg)
{
	struct adt_softc *sc = arg;
	u_int8_t cmd, data, data2;
	u_int16_t fan;
	int i, ratio;

	iic_acquire_bus(sc->sc_tag, 0);

	for (i = 0; i < sizeof worklist / sizeof(worklist[0]); i++) {

		if (sc->chip->ratio[worklist[i].index] == 0)
			continue;
		cmd = worklist[i].cmd;
		if (iic_exec(sc->sc_tag, I2C_OP_READ_WITH_STOP,
		    sc->sc_addr, &cmd, sizeof cmd, &data, sizeof data, 0)) {
			sc->sc_sensor[i].flags |= SENSOR_FINVALID;
			continue;
		}

		sc->sc_sensor[i].flags &= ~SENSOR_FINVALID;
		ratio = sc->chip->ratio[worklist[i].index];
		switch (worklist[i].sensor) {
		case ADT_VCC:
			if (sc->chip->vcc && (sc->sc_conf & ADT7460_CONFIG_Vcc))
				ratio = sc->chip->vcc;
			/* FALLTHROUGH */
		case ADT_2_5V:
		case ADT_VCCP:
		case ADT_V5:
		case ADT_V12:
			sc->sc_sensor[i].value = ratio * 1000 * (u_int)data / 192;
			break;
		case ADT_LOCAL_TEMP:
		case ADT_REM1_TEMP:
		case ADT_REM2_TEMP:
			if (data == 0x80)
				sc->sc_sensor[i].flags |= SENSOR_FINVALID;
			else
				sc->sc_sensor[i].value =
				    (int8_t)data * 1000000 + 273150000;
			break;
		case ADT_TACH1:
		case ADT_TACH2:
		case ADT_TACH3:
		case ADT_TACH4:
			cmd = worklist[i].cmd + 1; /* TACHnH follows TACHnL */
			if (iic_exec(sc->sc_tag, I2C_OP_READ_WITH_STOP,
			    sc->sc_addr, &cmd, sizeof cmd, &data2, sizeof data2, 0)) {
				sc->sc_sensor[i].flags |= SENSOR_FINVALID;
				continue;
			}

			fan = data + (data2 << 8);
			if (fan == 0 || fan == 0xffff)
				sc->sc_sensor[i].flags |= SENSOR_FINVALID;
			else
				sc->sc_sensor[i].value = (90000 * 60) / fan;
			break;
		default:
			sc->sc_sensor[i].flags |= SENSOR_FINVALID;
			break;
		}
	}

	iic_release_bus(sc->sc_tag, 0);
}
