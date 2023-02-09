import json
import base64
import requests
import time
from flask import Flask, jsonify, request
from flask_cors import CORS

app = Flask(__name__)
CORS(app, supports_credentials=True, resources=r"/*")


@app.route('/local_recv', methods=['POST', 'GET'])
def recv():
    with open("" + str(time.localtime().tm_min) + "." + str(time.localtime().tm_sec) + ".jpg", "wb") as f:
        f.write(request.data)
    img = request.data
    result = requests.post('http://127.0.0.1:24401/', params={'threshold': 0.5},
                           data=img).json()
    res = json.dumps(result)
    confidence = json.loads(res)["results"][0]["confidence"]
    print(confidence)
    if confidence > 0.5:
        requests.get(url='http://192.168.4.1/gpio/0')  # 执行程序

    return jsonify({"url": "ok"})


# @app.route('/local_detect', methods=['POST', 'GET'])
# def img_check():  # put application's code here
#
#     with open('./IMG/2.jpg', 'rb') as f:
#         img = f.read()
#     # params 为GET参数 data 为POST Body
#     result = requests.post('http://127.0.0.1:24401/', params={'threshold': 0.8},
#                            data=img).json()
#     res = json.dumps(result)
#     confidence = json.loads(res)["results"][0]["confidence"]
#
#     return str(confidence)


# @app.route('/cloud_detect', methods=['POST', 'GET'])
# def cloud_check():
#     # 目标图片的 本地文件路径，支持jpg/png/bmp格式
#     IMAGE_FILEPATH = "./IMG/3.jpg"
#     # 可选的请求参数
#     # threshold: 默认值为建议阈值，请在 我的模型-模型效果-完整评估结果-详细评估 查看建议阈值
#     PARAMS = {"threshold": 0.5}
#     # 服务详情 中的 接口地址
#     MODEL_API_URL = "https://aip.baidubce.com/rpc/2.0/ai_custom/v1/detection/catsearch01"
#
#     # 调用 API 需要 ACCESS_TOKEN。若已有 ACCESS_TOKEN 则于下方填入该字符串
#     # 否则，留空 ACCESS_TOKEN，于下方填入 该模型部署的 API_KEY 以及 SECRET_KEY，会自动申请并显示新 ACCESS_TOKEN
#     ACCESS_TOKEN = ""
#     API_KEY = "16IYCyPXEN4Nc3OyBhkkobkh"
#     SECRET_KEY = "gURSRZ21GIyH7rtw8k03z8Sd6hEK3uge"
#
#     with open(IMAGE_FILEPATH, 'rb') as f:
#         base64_data = base64.b64encode(f.read())
#         base64_str = base64_data.decode('UTF8')
#     PARAMS["image"] = base64_str
#
#     auth_url = "https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials" \
#                "&client_id={}&client_secret={}".format(API_KEY, SECRET_KEY)
#     auth_resp = requests.get(auth_url)
#     auth_resp_json = auth_resp.json()
#     ACCESS_TOKEN = auth_resp_json["access_token"]
#
#     request_url = "{}?access_token={}".format(MODEL_API_URL, ACCESS_TOKEN)
#     response = requests.post(url=request_url, json=PARAMS)
#     response_json = response.json()
#     response_str = json.dumps(response_json, indent=4, ensure_ascii=False)
#     print("结果:\n{}".format(response_str))
#
#     return response_str


if __name__ == '__main__':
    app.run()
