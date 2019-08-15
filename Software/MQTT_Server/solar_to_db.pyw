#Connect to mqtt broker
#Subscribe to topics
#When message is received, connect to db and insert data
#!/usr/bin/python

import paho.mqtt.client as mqtt
import sqlite3
from sqlite3 import Error

mqtt_broker = "Your MQTT broker IP"
database = "Your_Database.db"

inverter_list = [0,0,0,0,0,0]
charge_controller_list = [0,0,0,0,0]
battery_list = [0,0,0]
auxiliary_list = [0,0,0,0,0]

def check_strings():
    global battery_list
    global inverter_list
    global auxiliary_list
    global charge_controller_list

    if all(battery_list):
        query_db("battery", battery_list)
        battery_list = [0,0,0]
    if all(inverter_list):
        query_db("inverter", inverter_list)
        inverter_list = [0,0,0,0,0,0]
    if all(auxiliary_list):
        query_db("auxiliary", auxiliary_list)
        auxiliary_list = [0,0,0,0,0]    
    if all(charge_controller_list):
        query_db("charge_controller", charge_controller_list)
        charge_controller_list = [0,0,0,0,0]


def on_connect(client, userdata, flags, rc):
    print("connected with result code: " + str(rc))
    client.subscribe("solar/#")
    client.subscribe("+/+/temperature")
    client.subscribe("+/+/pressure")
    client.subscribe("+/+/humidity")

def on_message(client, userdata, msg):
    string1 = "solar/battery/amphours"
    string2 = "solar/battery/amps"
    string3 = "solar/auxiliary/amps"
    string4 = "solar/auxiliary/watts"
    string5 = "solar/auxiliary/volts"
    string6 = "solar/auxiliary/amphours"
    string7 = "solar/auxiliary/watthours"
    string8 = "solar/inverter/amps"
    string9 = "solar/inverter/watts"
    string10 = "solar/inverter/volts"
    string11 = "solar/inverter/amphours"
    string12 = "solar/inverter/watthours"
    string13 = "solar/CC/amps"
    string14 = "solar/CC/watts"
    string15 = "solar/CC/volts"
    string16 = "solar/CC/amphours"
    string17 = "solar/CC/watthours"
    string18 = "solar/memory"

    if msg.topic == string1:
        battery_list[0]=msg.payload
    elif msg.topic == string2:
        battery_list[1]=msg.payload
    elif msg.topic == string3:
        auxiliary_list[0]=msg.payload
    elif msg.topic == string4:
        auxiliary_list[1]=msg.payload
    elif msg.topic == string5:
        auxiliary_list[2]=msg.payload
    elif msg.topic == string6:
        auxiliary_list[3]=msg.payload
    elif msg.topic == string7:
        auxiliary_list[4]=msg.payload
    elif msg.topic == string8:
        inverter_list[0]=msg.payload
    elif msg.topic == string9:
        inverter_list[1]=msg.payload
    elif msg.topic == string10:
        inverter_list[2]=msg.payload
    elif msg.topic == string11:
        inverter_list[3]=msg.payload
    elif msg.topic == string12:
        inverter_list[4]=msg.payload
    elif msg.topic == string13:
        charge_controller_list[0]=msg.payload
    elif msg.topic == string14:
        charge_controller_list[1]=msg.payload
    elif msg.topic == string15:
        charge_controller_list[2]=msg.payload
        battery_list[2]=msg.payload
    elif msg.topic == string16:
        charge_controller_list[3]=msg.payload
    elif msg.topic == string17:
        charge_controller_list[4]=msg.payload    
    elif msg.topic == string18:
        inverter_list[5] = msg.payload

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
