from gevent import pywsgi
import time
from flask import Flask, request, jsonify
import os
import json
import base64
import requests
import tensorflow as tf
import numpy as np
from tensorflow.keras.preprocessing import image


def predict(img_path):
    path = img_path
    img = image.load_img(path, target_size=(150, 150))

    x = image.img_to_array(img)
    x = np.expand_dims(x, axis=0)
    images = np.vstack([x])

    model_save_path = './checkpoint/catface.ckpt'
    model = tf.keras.models.Sequential([
        tf.keras.layers.Conv2D(16, (3, 3), activation='relu', input_shape=(150, 150, 3)),
        tf.keras.layers.MaxPooling2D(2, 2),
        tf.keras.layers.Conv2D(32, (3, 3), activation='relu'),
        tf.keras.layers.MaxPooling2D(2, 2),
        tf.keras.layers.Conv2D(64, (3, 3), activation='relu'),
        tf.keras.layers.MaxPooling2D(2, 2),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dense(512, activation='relu'),
        tf.keras.layers.Dense(1, activation='sigmoid')
    ])
    model.load_weights(model_save_path)

    classes = model.predict(images, batch_size=10)
    print(classes[0])
    if classes[0] > 0:
        return "not cat"
    else:
        return "cat"


def clear_folder(folder_path, max_files):
    file_count = len(os.listdir(folder_path))
    if file_count > max_files:
        print("file's amount exceeds ", max_files)
        for filename in os.listdir(folder_path):
            file_path = os.path.join(folder_path, filename)
            if os.path.isfile(file_path):
                os.remove(file_path)
            print("the folder has already cleared")


def cat_detect(img_filepath, top_num):
    IMAGE_FILEPATH = img_filepath
    PARAMS = {"top_num": 2}
    MODEL_API_URL = "https://aip.baidubce.com/rpc/2.0/ai_custom/v1/detection/catsearch01"
    ACCESS_TOKEN = ""
    API_KEY = "16IYCyPXEN4Nc3OyBhkkobkh"
    SECRET_KEY = "gURSRZ21GIyH7rtw8k03z8Sd6hEK3uge"

    with open(IMAGE_FILEPATH, 'rb') as f:
        base64_data = base64.b64encode(f.read())
        base64_str = base64_data.decode('UTF8')
    PARAMS["image"] = base64_str

    if not ACCESS_TOKEN:
        auth_url = "https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials" \
                   "&client_id={}&client_secret={}".format(API_KEY, SECRET_KEY)
        auth_resp = requests.get(auth_url)
        auth_resp_json = auth_resp.json()
        ACCESS_TOKEN = auth_resp_json["access_token"]
    request_url = "{}?access_token={}".format(MODEL_API_URL, ACCESS_TOKEN)
    response = requests.post(url=request_url, json=PARAMS)
    response_json = response.json()
    #  response_str = json.dumps(response_json, indent=4, ensure_ascii=False)
    return response_json


app = Flask(__name__)


@app.route('/recv_img', methods=['GET', 'POST'])
def recv_img():  # put application's code here
    jpg_name = str(time.localtime().tm_min) + "." + str(time.localtime().tm_sec)
    with open("./img_tmp/" + jpg_name + ".jpg", "wb") as f:
        f.write(request.data)
    res_cloud = cat_detect("./img_tmp/" + jpg_name + ".jpg", 2)
    res_local = predict("./img_tmp/" + jpg_name + ".jpg")
    print(res_cloud['results'])
    empty = []
    if res_cloud['results'] != empty or res_local == "cat":
        requests.get(url="http://192.168.43.222:80/turning")
    clear_folder('./img_tmp', 10)
    return jsonify({"url": "ok"})


@app.route('/')
def welcome():
    print("hello there")
    return "hello there"


if __name__ == '__main__':
    server = pywsgi.WSGIServer(('0.0.0.0', 5000), app)
    server.serve_forever()
