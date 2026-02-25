import numpy as np
import os
from pathlib import Path
from sklearn.model_selection import train_test_split
import pickle

class PreparadorDatos:
    def __init__(self):
        self.raw_dir = Path("dataset/raw")
        self.processed_dir = Path("dataset/processed")
        self.processed_dir.mkdir(exist_ok=True)
        self.X = []
        self.y = []
        self.clases = []
        self.label_map = {}
        
    def cargar_datos(self):
        archivos = list(self.raw_dir.glob("*.npy"))
        print(f"📂 Encontrados {len(archivos)} archivos")
        for archivo in archivos:
            nombre_seña = archivo.stem.split('_')[0]
            if nombre_seña not in self.clases:
                self.clases.append(nombre_seña)
                print(f"  ✓ Nueva clase: {nombre_seña}")
            datos = np.load(archivo)
            print(f"  📊 {archivo.name}: {len(datos)} muestras")
            for muestra in datos:
                self.X.append(muestra)
                self.y.append(nombre_seña)
        print(f"\n✅ Total: {len(self.X)} muestras, {len(self.clases)} clases")
        
    def crear_mapeo(self):
        for i, clase in enumerate(self.clases):
            self.label_map[clase] = i
        y_numerico = [self.label_map[etiqueta] for etiqueta in self.y]
        return np.array(y_numerico)
    
    def dividir_datos(self, y_numerico):
        X_train, X_test, y_train, y_test = train_test_split(
            np.array(self.X), y_numerico, test_size=0.2,
            random_state=42, stratify=y_numerico
        )
        print(f"\n📊 División:")
        print(f"   Entrenamiento: {len(X_train)} muestras")
        print(f"   Prueba: {len(X_test)} muestras")
        return X_train, X_test, y_train, y_test
    
    def guardar_datos_procesados(self, X_train, X_test, y_train, y_test):
        np.save(self.processed_dir / "X_train.npy", X_train)
        np.save(self.processed_dir / "X_test.npy", X_test)
        np.save(self.processed_dir / "y_train.npy", y_train)
        np.save(self.processed_dir / "y_test.npy", y_test)
        with open(self.processed_dir / "label_map.pkl", "wb") as f:
            pickle.dump(self.label_map, f)
        print(f"\n💾 Datos guardados en {self.processed_dir}/")
        
    def ejecutar(self):
        print("🔄 PREPARANDO DATOS PARA ENTRENAMIENTO")
        print("="*50)
        self.cargar_datos()
        y_numerico = self.crear_mapeo()
        X_train, X_test, y_train, y_test = self.dividir_datos(y_numerico)
        self.guardar_datos_procesados(X_train, X_test, y_train, y_test)
        print("\n✅ ¡Datos listos para entrenar!")

if __name__ == "__main__":
    preparador = PreparadorDatos()
    preparador.ejecutar()