# Copyright (C) 2018  Toni Stachewicz, Niklas Hoffmann

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import ctypes
import json

from flask import Flask, request
from flask_cors import CORS, cross_origin

app = Flask(__name__)
lib = ctypes.cdll.LoadLibrary('libWritePadWrapper.so')

# config
cors = CORS(app, resources={"/": {"origins": "*"}})
app.config['CORS_HEADERS'] = 'Content-Type'
ENABLE_MULTIPLE_SUGGESTIONS = True


@app.route('/', methods=['POST'])
@cross_origin(supports_credentials=True, origin='*', headers=['Content-Type', 'Authorization'])
def handle_handwriting_recognition_request():
    try:
        json_points = request.json
        json_string = str(json_points).replace('\'', '\"')
        converted_for_cpp = ctypes.c_char_p(json_string.encode())

        if ENABLE_MULTIPLE_SUGGESTIONS:
            lib.recognizeMultipleSuggestions.restype = ctypes.c_wchar_p
            lib.recognizeMultipleSuggestions.argtypes = [ctypes.c_char_p]
            _result = lib.recognizeMultipleSuggestions(converted_for_cpp)
            result = str(_result)
        else:
            lib.recognizeSingleSuggestion.restype = ctypes.c_wchar_p
            lib.recognizeSingleSuggestion.argtypes = [ctypes.c_char_p]
            _result = lib.recognizeSingleSuggestion(converted_for_cpp)
            result = str(_result)
            result = json.dumps(result)

        response = app.response_class(
            response=result,
            status=200,
            mimetype='application/json'
        )
        return response
    except Exception as e:
        print(e)


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=9008)
