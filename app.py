from flask import Flask, request, jsonify
from db import database

app = Flask(__name__)

#db routes
@app.route("/init", methods=["GET"])
def init_db_route():
    database.init_db()
    return jsonify({"status": "ok"}), 201

# esp32 routes
@app.route('/ir/upload', methods=['POST'])
def ir_rcv_and_save():
    pass
    
@app.route('/ir/get_next', methods=['GET'])
def ir_get_next():
    pass

# web routes
@app.route('/remotes', methods=['GET'])
def get_remotes():
    data = database.get_all_remotes() 
    return jsonify(data), 200


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
        
        database.get_buttons(remote_id) 
        return jsonify({"status": "ok"}), 201
    
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


if __name__ == '__main__':
    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True
    )