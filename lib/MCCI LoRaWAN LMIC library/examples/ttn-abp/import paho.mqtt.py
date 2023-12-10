import paho.mqtt.client as mqtt

# Define the MQTT broker address and port
broker_address = "192.168.1.30"
port = 1883

# Define the MQTT topic to subscribe to
topic = "applications/1/devices/606f9d4b1227b524/data"  # Replace with your actual MQTT topic


# Callback when the client connects to the MQTT broker
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    # Subscribe to the specified topic
    client.subscribe(topic)


# Callback when a message is received from the MQTT broker
def on_message(client, userdata, msg):
    print(f"Received message on topic {msg.topic}: {msg.payload.decode('utf-8')}")


# Create an MQTT client
client = mqtt.Client()

# Set the callbacks
client.on_connect = on_connect
client.on_message = on_message

# Connect to the MQTT broker
client.connect(broker_address, port, 60)

# Loop to maintain the MQTT connection and process messages
client.loop_forever()
