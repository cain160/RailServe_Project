from flask import Flask, render_template, request, redirect
import sqlite3
import re

db = 'db/et_dev.sqlite'
conn = sqlite3.connect(db, check_same_thread=False)
cur = conn.execute('select * from Location_Info')

app = Flask(__name__)

####TESTING VARIABLES####
 
####TESTING VARIABLES####

@app.route('/')
@app.route('/index.html')
def home():
    return render_template('index.html')

@app.route('/details1.html')
def det_page1():
    return render_template('details1.html')

@app.route('/details2.html')
def det_page2():
    return render_template('details2.html')

@app.route('/details3.html')
def det_page3():
    return render_template('details3.html')
    
@app.route('/device', methods=['GET', 'POST'])
def device():
    startTime = request.form.get('start-time')
    endTime = request.form.get('end-time')
    device = request.form.get('device-search')
    
    device = int(device)
    if(device == 1):
        return redirect("details1.html")
    elif(device == 2):
        return redirect("details2.html")
    elif(device == 3):
        return redirect("details3.html")
        
@app.route('/getLocInf/<start>/<end>/<dev>', methods=['GET'])
def get_location_info(start,end,dev):
    startTime = start
    endTime = end
    device = dev
    
    cur.execute('select * from Location_Info where datetime(DateAndTime) > datetime(?) and datetime(DateAndTime) < datetime(?) and ModelNumber = ?', (startTime, endTime, device))
    rows = cur.fetchall()
    for row in rows:
        print('Ran get loc inf: ' + str(row))

    return 'GREAT'
    
def parse_dates_from_db():
    cur.execute('select DateAndTime from Location_Info where ModelNumber = 3')
    rows = cur.fetchall()   
    dateTimeList = []
    for date in rows:
        stringDate = str(date)
        dateTimeList.append(re.findall(r"[\w]+", stringDate))

if __name__ == '__main__':
    parse_dates_from_db()
    app.run(port=int("5000"), host='192.168.1.187', debug=True)

