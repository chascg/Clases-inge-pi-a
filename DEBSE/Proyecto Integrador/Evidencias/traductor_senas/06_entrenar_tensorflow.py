import numpy as np
import pickle
from pathlib import Path
from sklearn.model_selection import train_test_split
import tensorflow as tf
from tensorflow.keras import layers, models

class EntrenadorLSTM:
    def __init__(self):
        self.raw_dir = Path("dataset/raw")
        self.modelos_dir = Path("modelos")
        self.modelos_dir.mkdir(exist_ok=True)
        self.X = []
        self.y = []
        self.clases = []
        
    def cargar_datos(self):
        archivos = list(self.raw_dir.glob("*.npy"))
        print(f"📂 Encontrados {len(archivos)} archivos")
        for archivo in archivos:
            nombre_clase = archivo.stem.split('_')[0]
            if nombre_clase not in self.clases:
                self.clases.append(nombre_clase)
            datos = np.load(archivo)  # forma: (frames, 126)
            self.X.append(datos)
            self.y.append(nombre_clase)
            print(f"  {archivo.name}: {datos.shape}")
        
        # Convertir etiquetas a números
        self.label_map = {clase: i for i, clase in enumerate(self.clases)}
        y_numerico = [self.label_map[et] for et in self.y]
        
        # Padding para que todas las secuencias tengan la misma longitud (ej: 30 frames)
        # Podemos usar la longitud máxima
        max_len = max([seq.shape[0] for seq in self.X])
        X_padded = np.zeros((len(self.X), max_len, 126))
        for i, seq in enumerate(self.X):
            X_padded[i, :seq.shape[0], :] = seq
        
        # Dividir en train/test
        self.X_train, self.X_test, self.y_train, self.y_test = train_test_split(
            X_padded, y_numerico, test_size=0.2, random_state=42, stratify=y_numerico)
        
        print(f"\n✅ Datos listos: Train {self.X_train.shape}, Test {self.X_test.shape}")
        print(f"Clases: {self.clases}")
    
    def crear_modelo(self, max_len):
        model = models.Sequential([
            layers.Input(shape=(max_len, 126)),
            layers.LSTM(128, return_sequences=True),
            layers.Dropout(0.3),
            layers.LSTM(64),
            layers.Dropout(0.3),
            layers.Dense(32, activation='relu'),
            layers.Dense(len(self.clases), activation='softmax')
        ])
        model.compile(optimizer='adam',
                      loss='sparse_categorical_crossentropy',
                      metrics=['accuracy'])
        model.summary()
        return model
    
    def entrenar(self):
        max_len = self.X_train.shape[1]
        modelo = self.crear_modelo(max_len)
        
        history = modelo.fit(
            self.X_train, self.y_train,
            validation_split=0.2,
            epochs=50,
            batch_size=16,
            callbacks=[tf.keras.callbacks.EarlyStopping(patience=5, restore_best_weights=True)]
        )
        
        # Evaluar
        test_loss, test_acc = modelo.evaluate(self.X_test, self.y_test)
        print(f"\n✅ Precisión en prueba: {test_acc:.3f}")
        
        # Guardar modelo
        modelo.save(self.modelos_dir / "modelo_lstm.h5")
        with open(self.modelos_dir / "clases_lstm.pkl", "wb") as f:
            pickle.dump(self.clases, f)
        
        return modelo

if __name__ == "__main__":
    entrenador = EntrenadorLSTM()
    entrenador.cargar_datos()
    modelo = entrenador.entrenar()