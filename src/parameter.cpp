/**
 * Copyright (C) 2022 Axis Communications AB, Lund, Sweden
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "parameter.h"
#include <algorithm>
#include <regex>

using namespace std;

#define ERRORLOG cerr << "ERROR in Parameter: "
#define TRACELOG  \
    if (_verbose) \
    cout << "TRACE in Parameter: "

namespace acap_runtime {
Parameter::~Parameter() {
    if (ax_parameter != NULL)
        ax_parameter_free(ax_parameter);
}

// Initialize Parameter Service
bool Parameter::Init(const bool verbose) {
    _verbose = verbose;
    TRACELOG << "Init" << endl;
    _error = NULL;
    if (ax_parameter == NULL)
        ax_parameter = ax_parameter_new(APP_NAME, &_error);
    if (ax_parameter == NULL) {
        ERRORLOG << "Error when creating axparameter: " << _error->message << endl;
        g_clear_error(&_error);
        return false;
    }

    return true;
}

Status Parameter::GetValues(ServerContext* context, ServerReaderWriter<Response, Request>* stream) {
    Request request;
    Response response;
    while (stream->Read(&request)) {
        if (ax_parameter == NULL) {
            ERRORLOG << "axparameter not initalized" << endl;
            return Status::CANCELLED;
        }

        const gchar* parameter_key = request.key().c_str();
        const regex pattern("[a-zA-Z0-9.]+");
        if (!regex_match(parameter_key, pattern)) {
            TRACELOG << "No valid input request" << endl;
            return Status(StatusCode::INVALID_ARGUMENT, "No valid input request");
        }
        char* parameter_value = NULL;
        if (!ax_parameter_get(ax_parameter, parameter_key, &parameter_value, &_error)) {
            ERRORLOG << "Error when getting axparameter: " << _error->message << endl;
            parameter_value = g_strdup("");
            g_clear_error(&_error);
        }
        TRACELOG << parameter_key << ": " << parameter_value << endl;

        response.set_value(parameter_value);
        stream->Write(response);
        free(parameter_value);
    }

    return Status::OK;
}
}  // namespace acap_runtime
