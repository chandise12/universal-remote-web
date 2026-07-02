from flask import Flask, request, jsonify

app = Flask(__name__)

# esp32 routes
@app.route('/ir/upload', methods=['POST'])
def ir_rcv_and_save():
    pass
    
@app.route('/ir/get_next', methods=['GET'])
def ir_get_next():
    pass

# web routes
@app.route('/remotes', methods=['GET', 'POST'])
def remotes():
    pass

@app.route('/remotes/<remote_id>', methods=['GET', 'PUT', 'DELETE'])
def remote():
    pass

@app.route('/remotes/<remote_id>/buttons',  methods=['POST'])
def buttons():
    pass

@app.route('/remotes/<remote_id>/buttosn/<button_id>', methods=['GET', 'PUT', 'DELETE'])
def button():
    pass



if __name__ == '__main__':
    app.run(debug=True)