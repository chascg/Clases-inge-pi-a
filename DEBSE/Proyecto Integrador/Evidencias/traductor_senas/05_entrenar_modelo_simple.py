import numpy as np
import pickle
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report, confusion_matrix
import matplotlib.pyplot as plt
import seaborn as sns
from pathlib import Path

class EntrenadorSimple:
    def __init__(self):
        self.processed_dir = Path("dataset/processed")
        self.modelos_dir = Path("modelos")
        self.modelos_dir.mkdir(exist_ok=True)
        
    def cargar_datos(self):
        self.X_train = np.load(self.processed_dir / "X_train.npy")
        self.X_test = np.load(self.processed_dir / "X_test.npy")
        self.y_train = np.load(self.processed_dir / "y_train.npy")
        self.y_test = np.load(self.processed_dir / "y_test.npy")
        with open(self.processed_dir / "label_map.pkl", "rb") as f:
            self.label_map = pickle.load(f)
        self.clases = [None] * len(self.label_map)
        for nombre, idx in self.label_map.items():
            self.clases[idx] = nombre
        print(f"📊 Datos cargados: Train {self.X_train.shape}, Test {self.X_test.shape}")
        print(f"📋 Clases: {self.clases}")
        
    def entrenar(self):
        print("\n🌲 Entrenando Random Forest...")
        self.modelo = RandomForestClassifier(n_estimators=100, max_depth=15,
                                             min_samples_split=5, random_state=42, n_jobs=-1)
        self.modelo.fit(self.X_train, self.y_train)
        train_acc = self.modelo.score(self.X_train, self.y_train)
        test_acc = self.modelo.score(self.X_test, self.y_test)
        print(f"✅ Precisión entrenamiento: {train_acc:.3f}")
        print(f"✅ Precisión prueba: {test_acc:.3f}")
        y_pred = self.modelo.predict(self.X_test)
        print("\n📋 Reporte de clasificación:")
        print(classification_report(self.y_test, y_pred, target_names=self.clases))
        return y_pred
    
    def guardar_modelo(self):
        import joblib
        joblib.dump(self.modelo, self.modelos_dir / "modelo_random_forest.pkl")
        with open(self.modelos_dir / "clases.pkl", "wb") as f:
            pickle.dump(self.clases, f)
        print(f"\n💾 Modelo guardado en {self.modelos_dir}/")
    
    def visualizar_matriz_confusion(self, y_pred):
        cm = confusion_matrix(self.y_test, y_pred)
        plt.figure(figsize=(10,8))
        sns.heatmap(cm, annot=True, fmt='d', xticklabels=self.clases,
                   yticklabels=self.clases, cmap='Blues')
        plt.title('Matriz de Confusión')
        plt.ylabel('Real')
        plt.xlabel('Predicho')
        plt.tight_layout()
        plt.savefig(self.modelos_dir / 'matriz_confusion.png', dpi=150)
        plt.show()

if __name__ == "__main__":
    entrenador = EntrenadorSimple()
    entrenador.cargar_datos()
    y_pred = entrenador.entrenar()
    entrenador.guardar_modelo()
    entrenador.visualizar_matriz_confusion(y_pred)