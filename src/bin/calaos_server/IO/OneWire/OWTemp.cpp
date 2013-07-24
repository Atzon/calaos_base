/******************************************************************************
**  Copyright (c) 2007-2008, Calaos. All Rights Reserved.
**
**  This file is part of Calaos Home.
**
**  Calaos Home is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 3 of the License, or
**  (at your option) any later version.
**
**  Calaos Home is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Foobar; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**
******************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <OWTemp.h>
#include <ListeRule.h>
#include <IPC.h>

#ifdef HAVE_OWCAPI_H
#include <owcapi.h>
#endif

using namespace Calaos;

OWTemp::OWTemp(Params &p):
                Input(p),
                value(0.0),
                timer(0.0)
{
        std::string tmp;

        ow_id = get_param("ow_id");
        tmp = get_param("time");
        ow_args = get_param("ow_args");
        time = atof(tmp.c_str());

        printf("OW_ID : %s, time : %3.3f\n", ow_id.c_str(), time);

        if (!get_params().Exists("visible")) set_param("visible", "true");
        ListeRule::Instance().Add(this); //add this specific input to the EventLoop

#ifdef HAVE_OWCAPI_H
	char *res;
        size_t len;

        /* Read value */
        OW_init(ow_args.c_str());
        ow_req = ow_id + "/temperature";
        if(OW_get(ow_req.c_str(), &res, &len) >= 0)
        {
                value = atof(res);
                printf("Temperature read : %3.3f\n", value);
                free(res);
                Utils::logger("input") << Priority::INFO << "OWTemp::OWTemp(" << get_param("id") << "): Ok" << log4cpp::eol;
        }
        else
        {
                Utils::logger("input") << Priority::INFO << "OWTemp::OWTemp(" << get_param("id") << "): Cannot read One Wire Temperature Sensor (" << ow_id << ")" << log4cpp::eol;
        }
#else
        Utils::logger("input") << Priority::INFO << "OWTemp::OWTemp(" << get_param("id") << "): One Wire support not enabled !" << log4cpp::eol;
#endif


}

OWTemp::~OWTemp()
{
        Utils::logger("input") << Priority::INFO << "OWTemp::~OWTemp(): Ok" << log4cpp::eol;

#ifdef HAVE_OWCAPI_H
        OW_finish();
#endif
}

void OWTemp::hasChanged()
{
#ifdef HAVE_OWCAPI_H

        char *res;
        size_t len;
        double val = value;
        double sec = ecore_time_get() - timer;

        if (sec >= time)
        {
                timer = ecore_time_get();
                /* Read value */

                ow_req = ow_id + "/temperature";
                if(OW_get(ow_req.c_str(), &res, &len) >= 0)
                {
                        val = atof(res);
                        printf("Temperature read : %3.3f\n", val);
                        free(res);
                        Utils::logger("input") << Priority::INFO << "OWTemp::hasChanged(" << get_param("id") << "): Ok" << log4cpp::eol;
                }
                else
                {
                        Utils::logger("input") << Priority::INFO << "OWTemp::hasChanged(" << get_param("id") << "): Cannot read One Wire Temperature Sensor (" << ow_id << ")" << log4cpp::eol;
                }

                if (val != value)
                {
                        Utils::logger("input") << Priority::INFO << "OWTemp:changed(" << get_param("id") << ") : " << get_value_double() << " °C" << log4cpp::eol;

                        value = val;
                        EmitSignalInput();

                        string sig = "input ";
                        sig += get_param("id") + " ";
                        sig += Utils::url_encode(string("state:") + to_string(get_value_double()));
                        IPC::Instance().SendEvent("events", sig);
                }
        }
#endif
}

double OWTemp::get_value_double()
{
        return value;
}

void OWTemp::force_input_double(double v)
{
        value = v;
        EmitSignalInput();

        string sig = "input ";
        sig += get_param("id") + " ";
        sig += Utils::url_encode(string("state:") + to_string(get_value_double()));
        IPC::Instance().SendEvent("events", sig);
}