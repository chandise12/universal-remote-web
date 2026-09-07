from flask import Flask, request, jsonify, render_template
from db import database
import json

app = Flask(__name__)

esp_status = {"status": "IDLE",
              "remote_id": None,
              "button_id": None,
              "message": None}

user_status = {"status": "IDLE"}

# esp32 routes
@app.route('/ir/upload', methods=['POST'])
def ir_rcv_and_save():
    global esp_status

    data = request.get_json()

    if not data:
        return {"error": "no data"}, 400

    length = int(data.get("length"))
    remote_id = int(data.get("remote_id"))
    button_id = int(data.get("button_id"))
    message = json.dumps(data.get("message"))


    if length is None or remote_id is None or button_id is None or message is None:
        return {"error": "missing information"}, 400

    if length > 16: 
        database.update_message_button(remote_id, button_id, message)

    return jsonify(esp_status), 200

    
@app.route('/ir/get_state', methods=['GET'])
def ir_get_state():
    return jsonify(esp_status), 200


@app.route('/ir/clear_task', methods=['POST'])
def clear_task():

    global esp_status

    esp_status = {"status": "IDLE",
              "remote_id": None,
              "button_id": None,
              "message": None}

    return jsonify({"status": "ok"}), 200


#js user event routes
@app.route("/user_status", methods=["GET"])
def get_user_status():
    return jsonify(user_status), 200


# web routes
@app.route('/remotes', methods=['GET'])
def get_remotes():
    data = database.get_all_remotes() 
    print(data)
    return render_template('front_page.html', remotes=data)


@app.route('/remotes', methods=['POST'])
def create_remote():
    data = request.json 
    label = data.get("label") 
    
    if not label or label.strip() == "": 
        return jsonify({"status": "error"}), 400 
    
    database.create_remote(label) 
    return jsonify({"status": "ok"}), 201


@app.route('/remotes/<remote_id>', methods=['PUT'])
def update_remote(remote_id):
    try:
        data = request.json
        label = data.get("label") 

        if not label or label.strip() == "": 
            return jsonify({"status": "error"}), 400 

        database.update_label_remote(remote_id, label)
        return jsonify({"status": "ok"}), 200

    except ValueError:
        return jsonify({"status": "error", "message": "Remote not found"}), 404
    

@app.route('/remotes/<remote_id>', methods=['DELETE'])
def delete_remote(remote_id):
    try:
        database.delete_remote(remote_id) 
        return "", 204

    except ValueError:
        return jsonify({"status": "error"}), 400
   

@app.route('/remotes/<remote_id>/buttons', methods=['POST'])
def create_button(remote_id):
    try:    
        data = request.json 
        label = data.get("label") 
        
        if not label or label.strip() == "": 
            return jsonify({"status": "error"}), 400 
        
        database.create_button(remote_id, label) 
        return jsonify({"status": "ok"}), 201
    
    except ValueError:
        return jsonify({"status": "error", "message": "Remote not found"}), 404 


@app.route('/remotes/<remote_id>/buttons', methods=['GET'])
def get_buttons(remote_id):
    try:    
        buttons = database.get_buttons(remote_id) 
        remote = database.get_remote(remote_id)

        return render_template('remote.html', remote=remote, buttons=buttons)
    
    except ValueError:
        return jsonify({"status": "error", "message": "Remote not found"}), 404 


@app.route('/remotes/<remote_id>/buttons/<button_id>', methods=['PUT'])
def update_button(remote_id, button_id):
    try:    
        data = request.json
        label = data.get("label") 

        if not label or label.strip() == "": 
            return jsonify({"status": "error"}), 400 
        
        database.update_label_button(remote_id, button_id, label)
        return jsonify({"status": "ok"}), 200
    
    except ValueError:
        return jsonify({"status": "error", "message": "Remote/Button not found"}), 404 


@app.route('/remotes/<remote_id>/buttons/<button_id>', methods=['DELETE'])
def delete_button(remote_id, button_id):
    try:    
        database.delete_button(remote_id, button_id)
        return "", 204
    
    except ValueError:
        return jsonify({"status": "error", "message": "Remote/Button not found"}), 404 


@app.route('/remotes/<int:remote_id>/buttons/<int:button_id>/command', methods=['POST'])
def handle_button_command(remote_id, button_id):
    try:
        message = database.get_message_button(button_id, remote_id)

        global esp_status
        global user_status

        if message is None: # we want to read the signal from original remote
            esp_status["status"] = "LISTEN"
            esp_status["remote_id"] = remote_id
            esp_status["button_id"] = button_id

            user_status = jsonify({"status": "Please wait to press remote button"})
            
            return jsonify({"status": "message to be received"}), 200
        
        else: # we want to transmit the signal to the device
            # Convert string to list
            message = json.loads(message)

            esp_status["status"] = "TRANSMIT"
            esp_status["remote_id"] = remote_id
            esp_status["button_id"] = button_id
            esp_status["message"] = message

            user_status = jsonify({"status": "Please wait to press remote button"})
            
            return jsonify({"status": "message to be transmitted"}), 200

    except ValueError:
        return jsonify({"status": "error", "message": "Remote/Button not found"}), 404


if __name__ == '__main__':
    database.init_db()
    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True
    )