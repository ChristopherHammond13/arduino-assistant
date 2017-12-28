import base64
import json
import time

import requests

from paho.mqtt import client as mqtt

from secrets import (
    CLIENT_ID,
    CLIENT_SECRET,
    REFRESH_TOKEN,
    MQTT_SERVER,
    MQTT_PORT,
    MQTT_TOPIC
)

def on_mqtt_connect(client, user_data, flags, rc):
    print("Connected to MQTT with result code " + str(rc))
    client.subscribe(MQTT_TOPIC)

def on_message(client, user_data, msg):
    print("Message received on topic {}: {}".format(
        msg.topic,
        str(msg.payload)
    ))

def post_to_mqtt(client, playing, track, artist):
    data = {
        "playing": playing,
        "track": track,
        "artist": artist
    }
    payload = json.dumps(data)
    client.publish(
        MQTT_TOPIC,
        payload=payload,
        retain=True
    )


def get_bearer_token(current_token, expiry):
    if time.time() - expiry > 20:
        return (current_token, expiry)

    # We need a new token
    endpoint = "https://accounts.spotify.com/api/token"
    payload = {
        "grant_type": "refresh_token",
        "refresh_token": REFRESH_TOKEN,
        "client_id": CLIENT_ID,
        "client_secret": CLIENT_SECRET
    }
    # client_string = "{}:{}".format(CLIENT_ID, CLIENT_SECRET)
    # auth_string = base64.b64encode(bytes(client_string, 'utf-8'))
    # headers = {
    #     "Authorization": "Basic " + str(auth_string)
    # }
    r = requests.post(
        endpoint,
        data=payload,
        # headers=headers,
    )
    response = r.json()
    print(response)
    return (
        response["access_token"],
        int(time.time() + response["expires_in"])
    )


def get_now_playing(token):
    endpoint = "https://api.spotify.com/v1/me/player"
    headers = {
        "Authorization": "Bearer {}".format(token)
    }

    r = requests.get(
        endpoint,
        headers=headers
    )
    data = r.json()
    print(data)
    if data["is_playing"]:
        print("Now playing: {} by {}".format(
            data["item"]["name"],
            data["item"]["artists"][0]["name"]
        ))
        return (
            True,
            data["item"]["name"],
            data["item"]["artists"][0]["name"]
        )
    else:
        print("Not playing anything")
        return (False, None, None)


def poll(client, token):
    now_playing = get_now_playing(token)
    post_to_mqtt(
        client,
        now_playing[0],
        now_playing[1],
        now_playing[2]
    )


def start():
    client = mqtt.Client()
    client.on_connect = on_mqtt_connect
    client.on_message = on_message

    client.connect(MQTT_SERVER, MQTT_PORT, 60)
    client.loop_start()

    # Main blocking loop
    token = None
    expiry = 100000000000 # A very high number

    while 1:
        token, expiry = get_bearer_token(token, expiry)
        poll(client, token)
        time.sleep(1)


# Start polling when run
if __name__ == '__main__':
    start()
