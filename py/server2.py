from flask import Flask, render_template

app = Flask(__name__)

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

if __name__ == '__main__':
    app.run(port=int("5000"), host='192.168.1.187', debug=True)
