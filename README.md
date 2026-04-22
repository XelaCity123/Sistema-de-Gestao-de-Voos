# ✈️ IscteFlight – Sistema de Gestão de Voos (Operating Systems Project)

Projeto completo desenvolvido no âmbito da disciplina de **Sistemas Operativos**, que consiste na implementação de uma plataforma de gestão de reservas de voos, simulando o funcionamento de um sistema real distribuído.

Este projeto integra **três fases evolutivas**, abordando diferentes conceitos fundamentais de sistemas operativos, desde scripting até comunicação entre processos e concorrência.

---

## 📌 Descrição

O **IscteFlight** é um sistema que permite:
- Registo de passageiros  
- Compra de bilhetes de avião  
- Check-in de voos  
- Seleção de lugares  
- Gestão de reservas e voos  

O sistema foi desenvolvido de forma incremental, evoluindo de scripts simples para uma arquitetura **cliente-servidor concorrente com IPC** :contentReference[oaicite:0]{index=0}  

---

## 🧠 Arquitetura Geral

O sistema segue uma arquitetura progressiva:

1. **Gestão local com ficheiros (Shell Scripts)**
2. **Modelo Cliente-Servidor com processos e sinais**
3. **Sistema concorrente com memória partilhada e IPC**

Na fase final, o sistema inclui:
- Servidor principal  
- Servidores dedicados (processos filhos)  
- Clientes concorrentes  

com comunicação baseada em:
- Memória partilhada  
- Filas de mensagens  
- Semáforos  

:contentReference[oaicite:1]{index=1}  

---

## 🧩 Funcionalidades

### 👤 Gestão de Passageiros
- Registo de utilizadores
- Autenticação
- Gestão de saldo

### 🎫 Reservas de Voos
- Consulta de voos disponíveis  
- Compra de bilhetes  
- Atualização de lugares disponíveis  
- Registo de reservas  

### 🛫 Check-in
- Validação de credenciais  
- Associação a voo  
- Processamento concorrente de pedidos  

### 💺 Seleção de Lugares
- Escolha de lugar no avião  
- Atualização do estado do voo  
- Prevenção de conflitos concorrentes  

---

## ⚙️ Tecnologias e Conceitos

### 🐚 Parte 1 – Shell Scripts
- Bash scripting  
- Manipulação de ficheiros (`passageiros.txt`, `voos.txt`)  
- Validação de input  
- Processamento sequencial  

### 🔁 Parte 2 – Processos e Sinais
- `fork()` e `wait()`  
- Sinais (`SIGINT`, `SIGUSR1`, `SIGCHLD`, etc.)  
- Named Pipes (FIFO)  
- Comunicação entre processos  

:contentReference[oaicite:2]{index=2}  

### ⚡ Parte 3 – Concorrência e IPC
- Memória partilhada (`shmget`, `shmat`)  
- Semáforos (`semget`, `semop`)  
- Filas de mensagens (`msgget`, `msgsnd`)  
- Sincronização e exclusão mútua  

:contentReference[oaicite:3]{index=3}  

---

## 🧱 Estrutura do Projeto

```
.
├── parte-1/        # Scripts Bash
├── parte-2/        # Cliente-Servidor com sinais
├── parte-3/        # Sistema concorrente com IPC
├── passageiros.txt
├── voos.txt
├── relatorio_reservas.txt
└── README.md
```

---

## 🚀 Como executar

### Parte 1 (Shell Scripts)
```bash
./regista_passageiro.sh
./compra_bilhete.sh
```

### Parte 2 (Cliente-Servidor)
```bash
gcc servidor.c -o servidor
gcc cliente.c -o cliente

./servidor
./cliente
```

### Parte 3 (Sistema completo com IPC)
```bash
gcc servidor.c -o servidor
gcc cliente.c -o cliente

./servidor
./cliente
```

---

## 🧪 Conceitos Demonstrados

- Programação de sistemas  
- Concorrência entre processos  
- Sincronização e exclusão mútua  
- Comunicação entre processos (IPC)  
- Gestão de recursos partilhados  
- Arquitetura cliente-servidor  

---

## 📊 Objetivo Académico

Este projeto foi desenvolvido para consolidar conhecimentos fundamentais de Sistemas Operativos, incluindo:
- Gestão de processos  
- Comunicação entre processos  
- Sincronização  
- Manipulação de ficheiros  

---

## 👨‍💻 Autor

- Alexandre Duarte

---

## 📄 Licença

Projeto desenvolvido para fins académicos.
