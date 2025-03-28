from flask import Flask, jsonify, abort


app = Flask(__name__)


@app.route('/config/<uuid>', methods=['GET'])
def send_config(uuid):
    config = {
        "uuid": uuid,
        "mqttAddress": "mqtt://192.168.0.232:1883",
        "mqttUser": "testuser",
        "mqttPassword": "123456",
        "imageTopic": "image",
        "imageAckTopic": "image_ack",
        "healthReportTopic": "health",
        "healthReportRespTopic": "health_resp",
        "logTopic": "log",
        "cameraMode": "GRAY"
    }
    return jsonify(config)


""" @app.route('/config/<uuid>', methods=['GET'])
def send_400(uuid):
    abort(400) """


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=12534, ssl_context='adhoc')
