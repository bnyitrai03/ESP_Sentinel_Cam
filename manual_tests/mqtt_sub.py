# python 3.11
from paho.mqtt import client as mqtt_client
import logging
import json
from PIL import Image

broker = "192.168.0.232"
port = 1883
topic = "image"
ack_topic = "image_ack"
logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
                    datefmt='%Y-%m-%d %H:%M:%S',
                    handlers=[logging.StreamHandler()])

# Global variables to track state
expecting_image = False
last_timestamp = None


def connect_mqtt() -> mqtt_client.Client:
    def on_connect(client, userdata, flags, rc, properties=None):
        if rc == 0:
            logging.info("Connected to MQTT Broker!")
        else:
            logging.error(f"Failed to connect, return code {rc}")

    client = mqtt_client.Client(mqtt_client.CallbackAPIVersion.VERSION2)
    client.enable_logger()
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def subscribe(client: mqtt_client.Client):
    def on_message(client, userdata, msg):
        global expecting_image, last_timestamp
        try:
            # Attempt to decode JSON metadata
            payload = msg.payload.decode('utf-8').strip()
            doc = json.loads(payload)
            if 'timestamp' in doc and 'size' in doc:
                timestamp = doc['timestamp'][:20]
                logging.info(
                    f"Received metadata: Timestamp={timestamp}, Size={doc['size']}")
                client.publish(ack_topic, timestamp, qos=2)
                logging.info(f"Sent ACK for timestamp: {timestamp}")

                expecting_image = True
                last_timestamp = timestamp
                return
        except (UnicodeDecodeError, json.JSONDecodeError):
            pass  # Not a JSON message

        if expecting_image:
            try:
                if not last_timestamp:
                    logging.error("Image received but no timestamp available")
                    return
                safe_timestamp = last_timestamp.replace(
                    ":", "-").replace("T", "_")
                output_path = f"images/image_{safe_timestamp}.jpg"

                image = Image.frombytes("L", (2560, 1600), msg.payload)
                logging.info("Image decoded successfully")
                # image.save(output_path)
                # logging.info(f"Saved image to {output_path}")
                expecting_image = False
                last_timestamp = None
            except Exception as e:
                logging.error(f"Failed to process image: {e}")
                expecting_image = False
                last_timestamp = None
        else:
            logging.warning("Unexpected message: Not expecting image data")

    client.subscribe(topic)
    client.on_message = on_message


def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()


if __name__ == '__main__':
    run()
