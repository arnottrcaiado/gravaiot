# api para receber dados e gravar em banco MYSQL
#
from flask import Flask, request, render_template, url_for, redirect, json
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

# criação da classe com a estrutura da tabela para interacoes
class Interacoes(db.Model):
    id = db.Column(db.Integer, primary_key= True)
    modelo = db.Column(db.String(15))
    avalia = db.Column(db.Integer)
    data = db.Column(db.String(10))
    hora = db.Column(db.String(5))

    def to_json(self):
        return {"id": self.id, "modelo": self.modelo, "avalia": self.avalia, "data": self.data, "hora": self.hora }



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

@app.route('/configura', methods=['GET','POST'])
def configura():
    if request.method == 'GET' :
        return render_template( 'configura.html')
    elif request.method == 'POST' :
        minimo = request.form.get('minimo')
        maximo = request.form.get('maximo')
        # chama funcao que grava dados
        return {"Recebi dados ok": str(minimo)}

@app.route('/botoes', methods=['GET'])
def botoes():
    tt_botao1 = Interacoes.query.filter(Interacoes.modelo.like ("BT1"))
    tt_botao2 = Interacoes.query.filter(Interacoes.modelo.like ("BT2"))
    totais =[tt_botao1.count(),tt_botao2.count(), tt_botao1.count()+tt_botao2.count()]
    labels = ["Um", "Dois", "Geral"]
    return render_template('botoes.html', labels=labels, totais=totais, total_um = tt_botao1.count(), total_dois = tt_botao2.count())

@app.route('/botaoum', methods=['GET','POST'])
def botaoum():
    gravaDadosInteracoes( 'BT1', 10 )
    return redirect(url_for('botoes'))

@app.route('/botaodois', methods=['GET', 'POST'])
def botaodois():
    gravaDadosInteracoes( 'BT2', 10 )
    return redirect(url_for('botoes'))

@app.route('/mostraDados', methods=['GET'])
def mostraDados():
    consulta = Dados.query.all()
    dados = []
    for d in consulta :
        dados.append( {'id': d.id, 'sensor': d.sensor, 'valor': d.valor, 'data': d.data, 'hora': d.hora })
    return json.dumps( dados )
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

# grava dados das interacoes
def gravaDadosInteracoes( modelo, avaliacao ):
    data_atual = str(date.today())
    hora_atual = str(datetime.time(datetime.now()))
    hora_atual = hora_atual[0:5]
    dados=Interacoes(modelo=modelo, avalia=avaliacao, data=data_atual, hora=hora_atual)
    db.session.add(dados)
    db.session.commit()
    return