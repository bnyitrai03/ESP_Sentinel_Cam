# python 3.11
from paho.mqtt import client as mqtt_client
import base64
import logging
import json
from PIL import Image
import io
from datetime import datetime
import pytz

broker = "192.168.0.232"
port = 1883
topic = "mqtt/rpi/image"
logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
                    datefmt='%Y-%m-%d %H:%M:%S',
                    handlers=[logging.StreamHandler()])

logging.Formatter.converter = lambda *args: datetime.now(pytz.utc).timetuple()

def connect_mqtt() -> mqtt_client.Client:
    def on_connect(client, userdata, flags, rc, properties=None):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print(f"Failed to connect, return code {rc}")

    client = mqtt_client.Client(mqtt_client.CallbackAPIVersion.VERSION2)
    client.enable_logger()
    client.on_connect = on_connect
    client.connect(broker, port)
    return client

def subscribe(client: mqtt_client.Client):
    def on_message(client, userdata, msg):
        time = datetime.now(pytz.utc).strftime("%Y-%m-%d %H-%M-%S")

        try:
            """ # Parse the JSON message
            payload = json.loads(msg.payload)

            # Extract timestamp and image data
            image_base64 = payload['image']
            cpu_temp = payload['cpuTemp']
            battery_temp = payload['batteryTemp']
            battery_percentage = payload['batteryCharge'] """

            # Decode the Base64 image data
            image_data = base64.b64decode(msg.payload)
            image = Image.open(io.BytesIO(image_data))
            
            # Save directly to a file
            output_image_path = f"images/image_{time}.jpg"
            image.save(output_image_path)
            logging.info(f"Received and saved image as {output_image_path}")
            logging.info(f"Image size: {image._size}")
            logging.info(f"Time received: {time}")

            """ logging.info(f"The cpu temperature is: {cpu_temp} °C")
            logging.info(f"The battery temperature is: {battery_temp} °C")
            logging.info(f"The battery percentage is: {battery_percentage} %") """

        except Exception as e:
            logging.error(f"Failed to process image: {e}")
            logging.error(f"Payload: {msg.payload[:100]}...")

    client.subscribe(topic)
    client.on_message = on_message


def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()


if __name__ == '__main__':
    run()