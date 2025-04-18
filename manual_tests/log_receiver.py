from paho.mqtt import client as mqtt_client
import logging

logging_topic = "log"
qos = 2
broker = "192.168.0.232"
port = 1883

# Configure logging
logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s - %(levelname)s - %(message)s',
                    datefmt='%Y-%m-%d %H:%M:%S')


def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        logging.info(f"Connected with result code {rc}")
        logging.info(f"Subscribing to topic: {logging_topic} with QoS: {qos}")
        client.subscribe(logging_topic, qos)
        logging.info("Subscribed successfully")
    else:
        logging.error(f"Failed to connect, return code {rc}")


def on_message(client, userdata, msg):
    log_entry = msg.payload.decode()
    print(log_entry)


def connect_mqtt() -> mqtt_client.Client:
    client = mqtt_client.Client(mqtt_client.CallbackAPIVersion.VERSION2)
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(broker, port)
    return client


def run():
    client = connect_mqtt()
    client.loop_forever()


if __name__ == "__main__":
    run()
