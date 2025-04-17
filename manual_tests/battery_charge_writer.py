import json
from paho.mqtt import client as mqtt_client
import logging
import os

os.chdir(os.path.dirname(__file__))

broker = "192.168.0.232"
port = 1883
battery_current_topic = "battery_current"
battery_data_path = "battery_data.csv"

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


def subscribe(client):
    def on_message(client, userdata, msg):
        try:
            # Decode message
            message = msg.payload.decode()
            battery_data = json.loads(message)

            # Save battery percentage and uptime
            with open(battery_data_path, 'a') as file:
                file.write(
                    f"{battery_data['current']}, {battery_data['uptime']}\n")
            logging.info(f"Battery data: {battery_data}")

        except Exception as e:
            logging.error(f"An error occurred: {str(e)}")

    client.subscribe(battery_current_topic)
    client.on_message = on_message
    logging.info(f"Subscribed to topic: {battery_current_topic}")


def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()


if __name__ == '__main__':
    run()
