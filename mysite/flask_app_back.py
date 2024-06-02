# api para receber dados e gravar em banco MYSQL
#
from flask import Flask, request, render_template, url_for, redirect, json, jsonify
from flask_sqlalchemy import SQLAlchemy
import os
import time
from datetime import datetime, date
import pandas as pd
import csv

# Pasta onde os arquivos CSV serão armazenados
DATA_FOLDER = os.path.join('mysite', 'dados')
os.makedirs(DATA_FOLDER, exist_ok=True)

CSV_FILE_PATH = os.path.join(DATA_FOLDER, 'temperatures.csv')
CSV_LASTFILE_PATH = os.path.join(DATA_FOLDER, 'lasttemperature.csv')

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

# Dicionário original
categorias = {
    "produto1": "794613B2",
    "produto2": "536379D0600001",
    "comanda1": "29A08159",
    "comanda2": "D9BE345A",
    "produto5": "536A79D0600001",
    "multideia": "534179D0600001",
    "produto7": "535179D0600001",
    "produto8": "535879D0600001",
    "livro": "534B79D0600001",
    "medicamento": "534079D0600001",
    "carteira estudante": "53F5361C"
}

# Criar um DataFrame a partir do dicionário original
df_categorias = pd.DataFrame(list(categorias.items()), columns=['Nome', 'Codigo'])

# Inverter o DataFrame para facilitar a busca pelo código
df_codigos_para_nomes = df_categorias.set_index('Codigo')

# Função para obter o nome a partir do código usando o DataFrame
def obter_nome_pelo_codigo(codigo):
    try:
        return df_codigos_para_nomes.loc[codigo, 'Nome']
    except KeyError:
        return "Código não encontrado"


# criação da classe com a estrutura da tabela com dados de leitura sensores
class Dados(db.Model):
    id = db.Column(db.Integer, primary_key= True)
    sensor = db.Column(db.String(4))
    valor = db.Column(db.String(20))
    data = db.Column(db.String(10))
    hora = db.Column(db.String(5))

    def to_json(self):
        return {"id": self.id, "sensor": self.nome, "valor": self.email, "data": self.data, "hora": self.hora }

# criação da classe com a estrutura da tabela para interacoes
class Interacoes(db.Model):
    id = db.Column(db.Integer, primary_key= True)
    modelo = db.Column(db.String(20))
    avalia = db.Column(db.Integer)
    data = db.Column(db.String(10))
    hora = db.Column(db.String(5))

    def to_json(self):
        return {"id": self.id, "modelo": self.modelo, "avalia": self.avalia, "data": self.data, "hora": self.hora }

#--------------------------------------------

@app.route('/tabela', methods=['GET'])
def tabela():
    return render_template('tabela_dados.html')
# --------------------------------------------

@app.route('/maker', methods=['GET'])
def maker():
    return render_template('maker.html')

#-----------------------------------------

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
        return {"status": "erro-header invalido. ERR"}

statusModeloUm = "Null"

def initialize_csv_file(file_path):
    # Verifica se o arquivo já existe
    file_exists = os.path.isfile(file_path)

    # Se o arquivo não existe, cria e adiciona a linha de cabeçalho
    if not file_exists:
        with open(file_path, mode='w', newline='') as csv_file:
            writer = csv.writer(csv_file)
            # Cabeçalho com id, data, hora e 64 colunas de temperatura
            header = ['id', 'data', 'hora'] + [f'temp_{i}_{j}' for i in range(8) for j in range(8)]
            writer.writerow(header)

def is_data_equal(new_data, filename):
    last_line = read_last_line(filename)
    if last_line:
        return new_data == last_line
    return False

def save_data_to_csv(temperatures):
    csv_file_path = os.path.join(DATA_FOLDER, 'temperatures.csv')
    initialize_csv_file(csv_file_path)

    with open(csv_file_path, mode='a', newline='') as csv_file:
        writer = csv.writer(csv_file)
        # Gera um id único
        row_id = len(open(csv_file_path).readlines())
        # Data e hora atuais
        now = datetime.now()
        date_str = now.strftime('%Y-%m-%d')
        time_str = now.strftime('%H:%M:%S')
        # Escreve a linha com id, data, hora e as temperaturas
        row = [row_id, date_str, time_str] + temperatures
        writer.writerow(row)

def save_last_data_to_csv(temperatures):
    csv_file_path = os.path.join(DATA_FOLDER, 'lasttemperature.csv')
    initialize_csv_file(csv_file_path)

    with open(csv_file_path, mode='w', newline='') as csv_file:
        writer = csv.writer(csv_file)
        # Gera um id único
        row_id = len(open(csv_file_path).readlines())
        # Data e hora atuais
        now = datetime.now()
        date_str = now.strftime('%Y-%m-%d')
        time_str = now.strftime('%H:%M:%S')
        # Escreve a linha com id, data, hora e as temperaturas
        row = [row_id, date_str, time_str] + temperatures
        writer.writerow(row)


@app.route('/api/temperaturas', methods=['POST'])
def receive_temperatures():
    if request.is_json:
        data = request.get_json()
        if "valor" in data:
            temperatures = data["valor"]
            save_last_data_to_csv(temperatures)
            if not is_data_equal(temperatures, os.path.join(DATA_FOLDER, 'temperatures.csv')):
                save_data_to_csv(temperatures)
                return jsonify({"message": "Dados recebidos com sucesso", "dados": temperatures}), 200
            else:
                return jsonify({"message": "Dados não salvos, iguais"}), 200
        else:
            return jsonify({"error": "Dados inválidos"}), 400
    else:
        return jsonify({"error": "Requisição não é JSON"}), 400


@app.route('/api/latest-temperature')
def latest_temperature():
    last_line = read_last_line(CSV_LASTFILE_PATH)
    temp_data = last_line[3:]  # Ignora id, data, hora
    temperature_matrix = [temp_data[i:i + 8] for i in range(0, len(temp_data), 8)]
    return jsonify(temperature_matrix)

@app.route('/mapa')
def index():
    return render_template('mapacalor.html')

@app.route('/setamodelo', methods=['GET','POST'])
def setamodelo():
    dados = request.get_json()
    valor = dados["valor"]
    global statusModeloUm
    statusModeloUm = valor
    return {"Stat modelo um": statusModeloUm}

@app.route('/statusmodeloum', methods=['GET', 'POST'])
def testaget():
    global statusModeloUm
    valor = statusModeloUm
    statusModeloUm = "Null"
    return {"Modelo Um": str(valor)}

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
    modelos = ["Um", "Dois", "Geral"]
    return render_template('botoes.html', modelos=modelos, totais=totais, total_um = tt_botao1.count(), total_dois = tt_botao2.count())

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
        dados.append( {'id': d.id, 'sensor': d.sensor, 'valor': d.valor, 'item':obter_nome_pelo_codigo(d.valor), 'data': d.data, 'hora': d.hora })
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

def read_last_line(file_path):
    with open(file_path, 'r') as f:
        lines = f.readlines()
        last_line = lines[-1].strip().split(',')
        return last_line

if __name__ == '__main__':
    app.run(debug=True)
