import json
from paho.mqtt import client as mqtt_client
import logging
import os

os.chdir(os.path.dirname(__file__))

broker = "192.168.0.232"
port = 1883
health_rep_topic = "health"
config_topic = "health_resp"
config_path = "test_dynamic_config.json"

logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
                    datefmt='%Y-%m-%d %H:%M:%S',
                    handlers=[logging.StreamHandler()])


def connect_mqtt():
    def on_connect(client, userdata, flags, rc, properties=None):
        if rc == 0:
            logging.info("Connected to MQTT Broker!")
        else:
            logging.error(f"Failed to connect, return code {rc}")

    client = mqtt_client.Client(mqtt_client.CallbackAPIVersion.VERSION2)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def publish(client):
    try:
        with open(config_path, 'r') as file:
            config_data = json.load(file)

        # Convert the JSON object to a string
        message = json.dumps(config_data)
        # message = "config-ok"

        result = client.publish(config_topic, message, qos=2)
        result.wait_for_publish()
        status = result[0]
        if status == 0:
            logging.info(f"Sent config message!")
        else:
            logging.error(f"Failed to send message to topic {config_topic}")
    except FileNotFoundError:
        logging.error(f"Config file not found: {config_path}")
    except json.JSONDecodeError:
        logging.error(f"Invalid JSON in the config file: {config_path}")
    except Exception as e:
        logging.error(f"An error occurred: {str(e)}")


def run():
    client = connect_mqtt()
    client.loop_start()
    publish(client)
    client.disconnect()
    client.loop_stop()


if __name__ == '__main__':
    run()
