#!/bin/bash

# Cores para output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Configuração Automática de Secrets do GitHub (WSL/Headless) ===${NC}"

# Função para verificar se uma secret já existe
check_secret() {
    local secret_name=$1
    if gh secret list | grep -q "$secret_name"; then
        return 0 # Existe
    else
        return 1 # Não existe
    fi
}

# Opção de reset
FORCE_RESET=false
if [[ "$1" == "--reset" ]]; then
    FORCE_RESET=true
    echo -e "${YELLOW}⚠️  Modo de RESET ativado. Todas as secrets serão reconfiguradas.${NC}"
fi

# 1. Verificar Login no GitHub CLI
if ! gh auth status >/dev/null 2>&1; then
    echo -e "${YELLOW}⚠️  Você não está logado no GitHub CLI.${NC}"
    echo "Iniciando login via navegador (copie o código e abra a URL)..."
    gh auth login -p https -w
fi

# 2. Configurar Firebase (Automático via arquivo JSON)
echo ""
echo -e "${BLUE}🔍 Configurando Firebase...${NC}"
if [ "$FORCE_RESET" = false ] && check_secret "FIREBASE_SERVICE_ACCOUNT"; then
    echo -e "${GREEN}✅ FIREBASE_SERVICE_ACCOUNT já configurada. Pulando...${NC}"
else
    FIREBASE_FILE="terminalsnake-leaderboard-firebase-adminsdk-fbsvc-20e74338fa.json"
    if [ -f "$FIREBASE_FILE" ]; then
        echo -e "Arquivo encontrado: $FIREBASE_FILE"
        gh secret set FIREBASE_SERVICE_ACCOUNT < "$FIREBASE_FILE"
        echo -e "${GREEN}✅ FIREBASE_SERVICE_ACCOUNT configurada com sucesso!${NC}"
    else
        echo -e "${RED}❌ Arquivo $FIREBASE_FILE não encontrado na raiz.${NC}"
    fi
fi

# 3. Configurar Snapcraft (Interativo para gerar token)
echo ""
echo -e "${BLUE}🔐 Configurando Snap Store...${NC}"
if [ "$FORCE_RESET" = false ] && check_secret "SNAP_STORE_TOKEN"; then
    echo -e "${GREEN}✅ SNAP_STORE_TOKEN já configurada. Pulando...${NC}"
else
    if ! command -v snapcraft &> /dev/null; then
        echo -e "${YELLOW}Snapcraft não está instalado. Instalando...${NC}"
        sudo snap install snapcraft --classic
    fi

    echo "Gerando token de login do Snapcraft..."
    echo "Você precisará copiar o link gerado, abrir no navegador, autorizar e colar a resposta aqui se solicitado."

    rm -f snap_token.txt
    snapcraft export-login snap_token.txt --acls package_access,package_manage,package_push,package_release

    if [ -f "snap_token.txt" ]; then
        gh secret set SNAP_STORE_TOKEN < snap_token.txt
        rm snap_token.txt
        echo -e "${GREEN}✅ SNAP_STORE_TOKEN configurada com sucesso!${NC}"
    else
        echo -e "${RED}❌ Falha ao gerar o token do Snapcraft.${NC}"
    fi
fi

# 4. Configurar Homebrew (Manual - Token PAT)
echo ""
echo -e "${BLUE}🍺 Configurando Homebrew Tap Token${NC}"
if [ "$FORCE_RESET" = false ] && check_secret "HOMEBREW_TAP_TOKEN"; then
    echo -e "${GREEN}✅ HOMEBREW_TAP_TOKEN já configurada. Pulando...${NC}"
else
    echo -e "${YELLOW}Este token precisa ser criado manualmente no GitHub (Settings > Developer settings > PATs).${NC}"
    echo "Se você já tem o token, cole-o abaixo e pressione ENTER (ou deixe vazio para pular):"
    read -r brew_token

    if [ ! -z "$brew_token" ]; then
        echo "$brew_token" | gh secret set HOMEBREW_TAP_TOKEN
        echo -e "${GREEN}✅ HOMEBREW_TAP_TOKEN configurada!${NC}"
    else
        echo "⚠️  Pulo: Homebrew token não fornecido."
    fi
fi

# 5. Configurar Chocolatey (Manual - API Key)
echo ""
echo -e "${BLUE}🍫 Configurando Chocolatey API Key${NC}"
if [ "$FORCE_RESET" = false ] && check_secret "CHOCO_API_KEY"; then
    echo -e "${GREEN}✅ CHOCO_API_KEY já configurada. Pulando...${NC}"
else
    echo -e "${YELLOW}Obtenha sua chave em https://chocolatey.org/account${NC}"
    echo "Cole sua API Key abaixo e pressione ENTER (ou deixe vazio para pular):"
    read -r choco_key

    if [ ! -z "$choco_key" ]; then
        echo "$choco_key" | gh secret set CHOCO_API_KEY
        echo -e "${GREEN}✅ CHOCO_API_KEY configurada!${NC}"
    else
        echo "⚠️  Pulo: Chocolatey key não fornecida."
    fi
fi

echo ""
echo -e "${GREEN}=== Configuração Finalizada! ===${NC}"
echo ""
echo -e "${BLUE}🔍 Verificando secrets configuradas no GitHub:${NC}"
gh secret list
echo ""
echo "Dica: Para forçar a reconfiguração de todas as chaves, rode: ./setup_secrets.sh --reset"
