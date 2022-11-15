from flask import Flask, abort, jsonify, render_template, request
import json
import serial
app = Flask(__name__)

ser=serial.Serial('/dev/ttyAMA0', 115200, timeout=1)

temp_l = []
pres_l = []

@app.route('/')
def hello_world():
    return 'Hello, World!\n'

welcome = "Welcome to 3ESE API!"

@app.route('/api/welcome/', methods=['GET', 'POST', 'DELETE'])
def api_welcome():
    global welcome
    if request.method == 'POST':
        buffer = request.get_json()
        if "Sentence" in buffer:
            welcome = buffer["Sentence"]
            return "", 204
        else:
            return "No Sentence parameter", 400
    elif request.method == 'GET':
        return welcome
    elif request.method == 'DELETE':
        welcome = ""
        return "", 204


@app.route('/api/welcome/<int:index>')
def api_welcome_index(index):
    if index < len(welcome):
        return jsonify(welcome[index])
    else:
        abort(404)

@app.route('/api/temp/', methods=['GET', 'POST'])
def api_temp():
    if request.method == 'GET':
        temp_get = ""
        for i in temp_l:
            temp_get += str(i)
        return temp_get
    elif request.method == 'POST':
        ser.write(b"GET_T\r")
        new_temp = ser.readline()
        temp_l.append(new_temp)
        return "", 204

@app.route('/api/temp/<int:index>', methods=['GET', 'DELETE'])
def api_temp_index(index):
    if index < len(temp_l):
        if request.method == 'GET':
            return temp_l[index]
        elif request.method == 'DELETE':
            temp_l.pop(index)
            return "", 204
    else:
        abort(404)

@app.route('/api/pres/', methods=['GET', 'POST'])
def api_pres():
    if request.method == 'GET':
        pres_get = ""
        for i in pres_l:
            pres_get += str(i)
        return pres_get
    elif request.method == 'POST':
        ser.write(b"GET_P\r")
        new_pres = ser.readline()
        pres_l.append(new_pres)
        return "", 204

@app.route('/api/pres/<int:index>', methods=['GET', 'DELETE'])
def api_pres_index(index):
    if index < len(pres_l):
        if request.method == 'GET':
            return pres_l[index]
        elif request.method == 'DELETE':
            pres_l.pop(index)
            return "", 204
    else:
        abort(404)


@app.errorhandler(404)
def page_not_found(error):
    return render_template('page_not_found.html'), 404

@app.route('/api/request/', methods=['GET', 'POST'])
@app.route('/api/request/<path>', methods=['GET','POST'])
def api_request(path=None):
    resp = {
            "method":   request.method,
            "url" :  request.url,
            "path" : path,
            "args": request.args,
            "headers": dict(request.headers),
    }
    if request.method == 'POST':
        resp["POST"] = {
                "data" : request.get_json(),
                }
    return jsonify(resp)
