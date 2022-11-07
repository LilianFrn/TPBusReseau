from flask import Flask, abort, jsonify, render_template, request
import json
app = Flask(__name__)

@app.route('/')
def hello_world():
    return 'Hello, World!\n'

welcome = "Welcome to 3ESE API!"

@app.route('/api/welcome/', methods=['GET', 'POST', 'DELETE'])
def api_welcome():
    ret = welcome
    if request.method == 'POST':
        if request.data[1] == "Sentence":
            welcome = request.get_data()
    return jsonify(ret)

@app.route('/api/welcome/<int:index>', methods=['GET', 'PUT', 'DELETE', 'PATCH'])
def api_welcome_index(index):
    if request.method == 'POST':
    
    return jsonify(resp)


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

