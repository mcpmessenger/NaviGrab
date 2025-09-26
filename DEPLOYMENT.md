# 🚀 NaviGrab Deployment Guide

This guide covers deploying NaviGrab in various environments.

## 📋 Prerequisites

- Windows 10/11 (64-bit)
- Visual Studio 2022 Build Tools
- CMake 3.5+
- Git
- 4GB RAM minimum
- 2GB disk space

## 🏗️ Local Development Setup

### 1. Clone Repository
```bash
git clone https://github.com/mcpmessenger/NaviGrab.git
cd NaviGrab
```

### 2. Build Project
```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64

# Build project
cmake --build . --config Release
```

### 3. Run Examples
```bash
# Basic example
.\bin\Release\simple_example.exe

# Screenshot demo
.\bin\Release\screenshot_test_demo.exe

# Comprehensive demo
.\bin\Release\comprehensive_demo.exe
```

### 4. Launch Web Interface
Open `web_interface/navi_grab.html` in your browser.

## 🐳 Docker Deployment

### 1. Create Dockerfile
```dockerfile
FROM mcr.microsoft.com/windows/servercore:ltsc2022

# Install Visual Studio Build Tools
RUN powershell -Command \
    Invoke-WebRequest -Uri "https://aka.ms/vs/17/release/vs_buildtools.exe" -OutFile "vs_buildtools.exe" && \
    .\vs_buildtools.exe --quiet --wait --add Microsoft.VisualStudio.Workload.VCTools

# Install CMake
RUN powershell -Command \
    Invoke-WebRequest -Uri "https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1-windows-x86_64.msi" -OutFile "cmake.msi" && \
    msiexec /i cmake.msi /quiet

# Copy source code
COPY . /app
WORKDIR /app

# Build project
RUN cmake -B build -S . -G "Visual Studio 17 2022" -A x64
RUN cmake --build build --config Release

# Expose port
EXPOSE 8080

# Run application
CMD ["build\\bin\\Release\\navigrab.exe"]
```

### 2. Build and Run
```bash
# Build Docker image
docker build -t navigrab .

# Run container
docker run -p 8080:8080 navigrab
```

## ☸️ Kubernetes Deployment

### 1. Create Deployment YAML
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: navigrab
  labels:
    app: navigrab
spec:
  replicas: 3
  selector:
    matchLabels:
      app: navigrab
  template:
    metadata:
      labels:
        app: navigrab
    spec:
      containers:
      - name: navigrab
        image: navigrab:latest
        ports:
        - containerPort: 8080
        resources:
          requests:
            memory: "512Mi"
            cpu: "250m"
          limits:
            memory: "1Gi"
            cpu: "500m"
---
apiVersion: v1
kind: Service
metadata:
  name: navigrab-service
spec:
  selector:
    app: navigrab
  ports:
    - protocol: TCP
      port: 80
      targetPort: 8080
  type: LoadBalancer
```

### 2. Deploy to Kubernetes
```bash
# Apply deployment
kubectl apply -f navigrab-deployment.yaml

# Check status
kubectl get pods
kubectl get services
```

## ☁️ Cloud Deployment

### AWS ECS
```bash
# Build and push to ECR
aws ecr create-repository --repository-name navigrab
docker tag navigrab:latest 123456789012.dkr.ecr.us-west-2.amazonaws.com/navigrab:latest
docker push 123456789012.dkr.ecr.us-west-2.amazonaws.com/navigrab:latest

# Create ECS task definition
aws ecs register-task-definition --cli-input-json file://task-definition.json
```

### Azure Container Instances
```bash
# Create resource group
az group create --name navigrab-rg --location eastus

# Deploy container
az container create \
  --resource-group navigrab-rg \
  --name navigrab \
  --image navigrab:latest \
  --ports 8080 \
  --dns-name-label navigrab-unique
```

### Google Cloud Run
```bash
# Build and push to GCR
gcloud builds submit --tag gcr.io/PROJECT-ID/navigrab

# Deploy to Cloud Run
gcloud run deploy navigrab \
  --image gcr.io/PROJECT-ID/navigrab \
  --platform managed \
  --region us-central1 \
  --allow-unauthenticated
```

## 🔧 Configuration

### Environment Variables
```bash
# API Configuration
NAVIGRAB_API_PORT=8080
NAVIGRAB_API_HOST=0.0.0.0

# Screenshot Settings
NAVIGRAB_SCREENSHOT_FORMAT=png
NAVIGRAB_SCREENSHOT_QUALITY=90

# Scraping Settings
NAVIGRAB_MAX_DEPTH=3
NAVIGRAB_TIMEOUT=30000

# Storage Settings
NAVIGRAB_STORAGE_PATH=/data
NAVIGRAB_DB_URL=sqlite:///data/navigrab.db
```

### Configuration File
```json
{
  "api": {
    "port": 8080,
    "host": "0.0.0.0",
    "cors": {
      "enabled": true,
      "origins": ["*"]
    }
  },
  "screenshot": {
    "format": "png",
    "quality": 90,
    "max_width": 1920,
    "max_height": 1080
  },
  "scraping": {
    "max_depth": 3,
    "timeout": 30000,
    "user_agent": "NaviGrab/1.0"
  },
  "storage": {
    "path": "/data",
    "db_url": "sqlite:///data/navigrab.db"
  }
}
```

## 📊 Monitoring

### Health Checks
```bash
# Check API health
curl http://localhost:8080/health

# Check metrics
curl http://localhost:8080/metrics
```

### Logging
```bash
# View logs
docker logs navigrab-container

# Follow logs
docker logs -f navigrab-container
```

## 🔒 Security

### SSL/TLS Configuration
```bash
# Generate SSL certificate
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes

# Configure HTTPS
NAVIGRAB_SSL_CERT=cert.pem
NAVIGRAB_SSL_KEY=key.pem
```

### Authentication
```bash
# Enable API key authentication
NAVIGRAB_API_KEY=your-secret-key
NAVIGRAB_AUTH_ENABLED=true
```

## 🚀 Production Checklist

- [ ] SSL/TLS certificates configured
- [ ] Authentication enabled
- [ ] Monitoring and logging setup
- [ ] Backup strategy implemented
- [ ] Security headers configured
- [ ] Rate limiting enabled
- [ ] Error handling tested
- [ ] Performance testing completed
- [ ] Documentation updated
- [ ] Team training completed

## 📞 Support

For deployment issues:
1. Check the troubleshooting guide
2. Review logs and metrics
3. Open an issue on GitHub
4. Contact the development team

---

**Status**: 🚧 In Development | **Version**: 1.0.0 | **Last Updated**: 2024-01-01
