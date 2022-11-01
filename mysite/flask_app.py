# api para receber dados e gravar em banco MYSQL
#
from flask import Flask, request, render_template, url_for, redirect
from flask_sqlalchemy import SQLAlchemy
import os
import time
from datetime import datetime, date


os.environ["TZ"] = "America/Recife"
time.tzset()

header_key = 'eFgHjukoli12Reatyghmaly76'


# configuração base do sqlalchemy
app = Flask(__name__)
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = True
app.config['SQLALCHEMY_DATABASE_URI'] = 'mysql://gravaiot:Aa123456789@gravaiot.mysql.pythonanywhere-services.com/gravaiot$dadossensor'
db = SQLAlchemy(app)

# etapa 1 - criação do db
# no console python
# >>> from flask_app import db
# >>> db.create_all()
# flask db init
#

# criação da classe com a estrutura da tabela Usuario
class Dados(db.Model):
    id = db.Column(db.Integer, primary_key= True)
    sensor = db.Column(db.String(4))
    valor = db.Column(db.String(10))
    data = db.Column(db.String(10))
    hora = db.Column(db.String(5))

    def to_json(self):
        return {"id": self.id, "sensor": self.nome, "valor": self.email, "data": self.data, "hora": self.hora }


@app.route('/postJson', methods=['POST'])
def postJson():
    cab = request.headers.get('Authorization-Token')    # token que pode ser utilizado para validacao
    if validaHeader(cab) :
        dados = request.get_json()                          # recebe os dados em formato json
        sensor  = dados["sensor"]                           # identifica cada elemento do json de origem
        valor = dados["valor"]
        gravaDadosDb ( sensor, valor )
        return {"status": str(sensor) , "valor": str(valor)}
    else :
        return {"status": "erro-header invalido"}

@app.route('/mostraDados', methods=['GET'])
def mostraDados():
    consulta = Dados.query.all()
    dados = []
    for d in consulta :
        dados.append( {'id': d.id, 'sensor': d.sensor, 'valor': d.valor, 'data': d.data, 'hora': d.hora })
    return dados
# funcao para validar header
def validaHeader( cabecalho ):
    if cabecalho == header_key :
        return True
    else :
        return False

# grava dados
def gravaDadosDb( sensor, valor ):
    data_atual = str(date.today())
    hora_atual = str(datetime.time(datetime.now()))
    hora_atual = hora_atual[0:5]
    dados=Dados(sensor=sensor, valor=valor, data=data_atual, hora=hora_atual)
    db.session.add(dados)
    db.session.commit()
    return