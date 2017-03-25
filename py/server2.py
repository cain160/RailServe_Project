from flask import Flask, render_template
import sqlite3
import re

db = 'db/et_dev.sqlite'
conn = sqlite3.connect(db)
cur = conn.execute('select * from Location_Info')

app = Flask(__name__)

####TESTING VARIABLES####
#
#
#
#
modNum = 3
#
#
#
#
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
    
def parse_dates_from_db():
    cur.execute('select DateAndTime from Location_Info where ModelNumber = ' + str(modNum))
    rows = cur.fetchall()
    dateTimeList = []
    for row in rows:
        stringRow = str(row)
        dateTimeList.append(re.findall(r"[\w]+", stringRow))

if __name__ == '__main__':
    rows = cur.fetchall()
    parse_dates_from_db()
    app.run(port=int("5000"), host='192.168.1.187', debug=True)

