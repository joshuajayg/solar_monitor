#Connect to mqtt broker
#Subscribe to topics
#When message is received, connect to db and insert data
#!/usr/bin/env python

import paho.mqtt.client as mqtt
import sqlite3
from sqlite3 import Error

mqtt_broker = "192.168.4.1"
database = "/media/pi/DATABASE/SQLite_and_Paho/Atmosphere.db"

indoor_list = [0,0,0,0]
outdoor_list = [0]


def check_strings():
    global indoor_list
    global outdoor_list

    if all(indoor_list):
        query_db("indoor", indoor_list)
        indoor_list = [0,0,0,0]
    if all(outdoor_list):
        query_db("outdoor", outdoor_list)
        outdoor_list = [0]


def on_connect(client, userdata, flags, rc):
    print("connected with result code: " + str(rc))
    client.subscribe("indoor/#")
    client.subscribe("outdoor/#")

def on_message(client, userdata, msg):
    string1 = "indoor/upstairs/temperature"
    string2 = "indoor/downstairs/temperature"
    string3 = "indoor/downstairs/humidity"
    string4 = "indoor/downstairs/pressure"
    string5 = "outdoor/east/temperature"
    

    if msg.topic == string1:
        indoor_list[0]=msg.payload
    elif msg.topic == string2:
        indoor_list[1]=msg.payload
    elif msg.topic == string3:
        indoor_list[2]=msg.payload
    elif msg.topic == string4:
        indoor_list[3]=msg.payload
    elif msg.topic == string5:
        outdoor_list[0]=msg.payload

    try:
        check_strings()
    except:
        pass    


def query_db(table, payload):
    query_string = ""
    for item in payload:
        query_string = query_string + ", " + item
    conn = create_connection(database)
    with conn:
        cur = conn.cursor()
        cur.execute("INSERT INTO " + table + " VALUES (datetime('now')" + query_string + ")")
        print("Query executed for " + table + ".")
        conn.commit()
        print("Query successful.")
def create_connection(db_file):
    try: 
        conn = sqlite3.connect(db_file)
        print(conn)
        return conn
    except Error as e:
        print(e)

    return None

def main():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(mqtt_broker, 1883, 60)

    client.loop_forever()

if __name__ == '__main__':
    main()
