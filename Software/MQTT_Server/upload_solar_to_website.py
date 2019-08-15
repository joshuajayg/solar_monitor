# connect to server and get last rowid from database
# access local database and check for rows higher than server database
# if more rows on local database, download those rows
# store row data as json and upload to server using POST

###  TODO  ###
#Current setup only update 'outdoor' table
#Include variables to update 'indoor' table
#Update server-side php docs to reflect multiple tables
#Send table being updated through POST/GET request
#Update both tables in one json string


import sqlite3
from sqlite3 import Error
from time import sleep
from time import localtime, strftime
import requests
import json

local_solar_database = "Your_Local_Database.db"
get_request = "Your_Remote_Server_GET_URL" 
post_request = "Your_Remote_Server_POST_URL"
key = 'YOUR_KEY'  #my "super secret" key to make sure I only get my data
tables = ("charge_controller", "battery", "inverter", "auxiliary")

def create_connection(db_file):
    try:
        conn = sqlite3.connect(db_file)
        print(conn)
        return conn
    except Error as e:
        print(e)

    return None

def check_server(query_table):  #get the highest ROWID from the server's database
    print 'checking server for max ROWID'
    GET_DESTINATION = get_request
    data = {
        'table':query_table,
        'key':key,
        'request':'MAX(ROWID)',
    }

    r = requests.get(url = GET_DESTINATION, params = data)
    if (r.status_code==200):
        print("request Success")
        print(r.text)
        result = r.json()
    elif (r.status_code==500):
        print("error")
        exit
        
    maxROWID = result['MAX(ROWID)']
    print 'Done!  Max server ROWID: ', maxROWID
    return maxROWID  #return the highest ROWID to main
    
def check_db(query_table):
    print 'checking local database for max ROWID'
    conn = create_connection(local_solar_database)
    with conn:
        conn.row_factory = sqlite3.Row
        cur = conn.cursor()
        query = 'SELECT MAX(ROWID) FROM ' + query_table
        print(query)
        cur.execute(query)
        r = cur.fetchone()
        maxROWID = r['MAX(ROWID)']
        print 'Done! Max local ROWID: ', maxROWID
        return maxROWID

def query_db(maxROWID_server, table):
    conn = create_connection(local_solar_database)
    with conn:
        conn.row_factory = sqlite3.Row
        cur = conn.cursor()
        if (table == 'charge_controller'):
            query = "SELECT time, amps, watts, volts, amphours, watthours FROM charge_controller WHERE ROWID > " + str(maxROWID_server)
            cur.execute(query)
            r = cur.fetchall()
            print 'converting data to json'
            items = []
            for row in r:
                items.append({'time': row[0], 'amps': row[1], 'watts': row[2], 'volts': row[3], 'amphours': row[4], 'watthours': row[5]})

            data_json = json.dumps({'items': items, 'key': key, 'table': table})
            print 'Done converting.'
        elif (table == 'battery'):
            query = "SELECT time, amphours, amps, volts FROM battery WHERE ROWID> " + str(maxROWID_server,)
            cur.execute(query)
            r = cur.fetchall()
            print 'converting data to json'
            items = []
            for row in r:
                items.append({'time': row[0], 'amphours': row[1], 'amps': row[2], 'volts': row[3]})

            data_json = json.dumps({'items': items, 'key': key, 'table': table})
            print 'Done converting.'
        elif (table == 'inverter'):
            query = "SELECT time, amps, watts, volts, amphours, watthours, memory FROM inverter WHERE ROWID> " + str(maxROWID_server,)
            cur.execute(query)
            r = cur.fetchall()
            print 'converting data to json'
            items = []
            for row in r:
                items.append({'time': row[0], 'amps': row[1], 'watts': row[2], 'volts': row[3], 'amphours': row[4], 'watthours': row[5], 'memory': row[6]})

            data_json = json.dumps({'items': items, 'key': key, 'table': table})
            print 'Done converting.'
        elif (table == 'auxiliary'):
            query = "SELECT time, amps, watts, volts, amphours, watthours FROM auxiliary WHERE ROWID> " + str(maxROWID_server,)
            cur.execute(query)
            r = cur.fetchall()
            print 'converting data to json'
            items = []
            for row in r:
                items.append({'time': row[0], 'amps': row[1], 'watts': row[2], 'volts': row[3], 'amphours': row[4], 'watthours': row[5]})

            data_json = json.dumps({'items': items, 'key': key, 'table': table})
            print 'Done converting.'
            
        return data_json

def update_server(data_array):
    POST_DESTINATION = post_request
    r = requests.post(url = POST_DESTINATION, data = data_array)
    print 'server response: ', r.status_code
    print ''
def main():
    while(1):
        try:
            for table in tables:
                print('Updating ', table, ' table.........')
                maxROWID_server = check_server(table)  #check server for highest ROWID in database
                maxROWID_local = check_db(table)  #check for new rows on local database
                if(maxROWID_local>maxROWID_server):
                    print 'Time to update the server...'
                    data_json = query_db(maxROWID_server, table)  #get all new rows from local database
                    print('printing data_json')
                    print(data_json)
                    update_server(data_json)
            sleep(60*5)
        except Exception as e:
            print e, '; trying again in 1 minute...'
            sleep(60)

if __name__ == '__main__':
    main()
