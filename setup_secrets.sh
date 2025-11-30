#!/bin/bash

# Cores para output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Configuração Automática de Secrets do GitHub (WSL/Headless) ===${NC}"

# 1. Verificar Login no GitHub CLI
if ! gh auth status >/dev/null 2>&1; then
    echo -e "${YELLOW}⚠️  Você não está logado no GitHub CLI.${NC}"
    echo "Iniciando login via navegador (copie o código e abra a URL)..."
    gh auth login -p https -w
fi

# 2. Configurar Firebase (Automático via arquivo JSON)
echo ""
echo -e "${BLUE}🔍 Procurando arquivo de credenciais do Firebase...${NC}"
FIREBASE_FILE="terminalsnake-leaderboard-firebase-adminsdk-fbsvc-20e74338fa.json"

if [ -f "$FIREBASE_FILE" ]; then
    echo -e "Arquivo encontrado: $FIREBASE_FILE"
    gh secret set FIREBASE_SERVICE_ACCOUNT < "$FIREBASE_FILE"
    echo -e "${GREEN}✅ FIREBASE_SERVICE_ACCOUNT configurada com sucesso!${NC}"
else
    echo -e "${RED}❌ Arquivo $FIREBASE_FILE não encontrado na raiz.${NC}"
    echo "Certifique-se de que o arquivo está na pasta atual."
fi

# 3. Configurar Snapcraft (Interativo para gerar token)
echo ""
echo -e "${BLUE}🔐 Configurando Snap Store Token...${NC}"

if ! command -v snapcraft &> /dev/null; then
    echo -e "${YELLOW}Snapcraft não está instalado. Instalando...${NC}"
    sudo snap install snapcraft --classic
fi

echo "Gerando token de login do Snapcraft..."
echo "Você precisará copiar o link gerado, abrir no navegador, autorizar e colar a resposta aqui se solicitado."


# Remove arquivo antigo se existir
rm -f snap_token.txt

# Executa export-login. No modo headless, ele deve fornecer uma URL.
# O output é salvo em snap_token.txt
snapcraft export-login snap_token.txt --acls package_access,package_manage,package_push,package_release

if [ -f "snap_token.txt" ]; then
    gh secret set SNAP_STORE_TOKEN < snap_token.txt
    rm snap_token.txt
    echo -e "${GREEN}✅ SNAP_STORE_TOKEN configurada com sucesso!${NC}"
else
    echo -e "${RED}❌ Falha ao gerar o token do Snapcraft.${NC}"
fi

# 4. Configurar Homebrew (Manual - Token PAT)
echo ""
echo -e "${BLUE}🍺 Configurando Homebrew Tap Token${NC}"
echo -e "${YELLOW}Este token precisa ser criado manualmente no GitHub (Settings > Developer settings > PATs).${NC}"
echo "Se você já tem o token, cole-o abaixo e pressione ENTER (ou deixe vazio para pular):"
read -r brew_token

if [ ! -z "$brew_token" ]; then
    echo "$brew_token" | gh secret set HOMEBREW_TAP_TOKEN
    echo -e "${GREEN}✅ HOMEBREW_TAP_TOKEN configurada!${NC}"
else
    echo "⚠️  Pulo: Homebrew token não fornecido."
fi

# 5. Configurar Chocolatey (Manual - API Key)
echo ""
echo -e "${BLUE}🍫 Configurando Chocolatey API Key${NC}"
echo -e "${YELLOW}Obtenha sua chave em https://chocolatey.org/account${NC}"
echo "Cole sua API Key abaixo e pressione ENTER (ou deixe vazio para pular):"
read -r choco_key

if [ ! -z "$choco_key" ]; then
    echo "$choco_key" | gh secret set CHOCO_API_KEY
    echo -e "${GREEN}✅ CHOCO_API_KEY configurada!${NC}"
else
    echo "⚠️  Pulo: Chocolatey key não fornecida."
fi

echo ""
echo -e "${GREEN}=== Configuração Finalizada! ===${NC}"
