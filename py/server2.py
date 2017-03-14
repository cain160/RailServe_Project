from flask import Flask, render_template

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('details1.html')

if __name__ == '__main__':
    app.run(port=int("5000"), host='192.168.1.187', debug=True)
