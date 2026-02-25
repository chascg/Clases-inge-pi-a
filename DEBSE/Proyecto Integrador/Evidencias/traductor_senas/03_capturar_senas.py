import cv2
import mediapipe as mp
import numpy as np
import time
from pathlib import Path
from typing import Optional, List


class Config:
    """Configuración centralizada de constantes."""
    # Parámetros de cámara
    FRAME_WIDTH = 640
    FRAME_HEIGHT = 480
    
    # Parámetros de MediaPipe Hands
    HANDS_STATIC_MODE = False
    HANDS_MAX_NUM = 2
    HANDS_MIN_DETECTION_CONFIDENCE = 0.5
    HANDS_MIN_TRACKING_CONFIDENCE = 0.5
    
    # Parámetros de landmarks
    LANDMARKS_PER_HAND = 21  # Puntos por mano
    COORDINATES_PER_LANDMARK = 3  # x, y, z
    LANDMARKS_PER_HAND_TOTAL = LANDMARKS_PER_HAND * COORDINATES_PER_LANDMARK  # 63
    LANDMARKS_BOTH_HANDS = LANDMARKS_PER_HAND_TOTAL * 2  # 126
    
    # Parámetros de UI
    FONT = cv2.FONT_HERSHEY_SIMPLEX
    FONT_SCALE_INFO = 0.8
    FONT_SCALE_STATUS = 0.7
    FONT_SCALE_COUNTDOWN = 4.0
    TEXT_COLOR_SUCCESS = (0, 255, 0)
    TEXT_COLOR_WARNING = (255, 255, 0)
    TEXT_COLOR_ERROR = (0, 0, 255)
    TEXT_COLOR_COUNTDOWN = (0, 255, 255)
    TEXT_THICKNESS = 2
    
    # Parámetros de captura
    DEFAULT_FPS = 10
    DEFAULT_DURATION_STATIC = 1
    DEFAULT_DURATION_DYNAMIC = 3
    DEFAULT_SAMPLES_PER_CLASS = 100
    
    # Rutas
    DEFAULT_DATASET_PATH = Path("dataset/raw")


class HandLandmarkExtractor:
    """Extrae y normaliza landmarks de manos desde resultados de MediaPipe."""
    
    def __init__(self, max_hands: int = 2):
        self.max_hands = max_hands
        self.landmarks_per_hand = Config.LANDMARKS_PER_HAND_TOTAL
        self.landmarks_total = Config.LANDMARKS_BOTH_HANDS
    
    def extract_both_hands(self, results) -> np.ndarray:
        """
        Extrae landmarks de hasta dos manos.
        
        Returns:
            Array de 126 elementos (2 manos * 63 coordenadas) o ceros si no hay detección.
        """
        landmarks = []
        
        if not results.multi_hand_landmarks:
            return np.zeros(self.landmarks_total, dtype=np.float32)
        
        # Procesar primera mano
        if len(results.multi_hand_landmarks) >= 1:
            landmarks.extend(self._flatten_landmarks(results.multi_hand_landmarks[0]))
        else:
            landmarks.extend([0.0] * self.landmarks_per_hand)
        
        # Procesar segunda mano
        if len(results.multi_hand_landmarks) >= 2:
            landmarks.extend(self._flatten_landmarks(results.multi_hand_landmarks[1]))
        else:
            landmarks.extend([0.0] * self.landmarks_per_hand)
        
        return np.array(landmarks, dtype=np.float32)
    
    def _flatten_landmarks(self, hand_landmarks) -> List[float]:
        """Convierte landmarks de una mano en lista plana de coordenadas."""
        coords = []
        for landmark in hand_landmarks.landmark:
            coords.extend([landmark.x, landmark.y, landmark.z])
        return coords


class SampleCapturer:
    """Maneja la captura y guardado de muestras de video/landmarks."""
    
    def __init__(self, output_dir: Path, fps: int = Config.DEFAULT_FPS):
        self.output_dir = output_dir
        self.output_dir.mkdir(parents=True, exist_ok=True)
        self.fps = fps
        self.landmark_extractor = HandLandmarkExtractor()
    
    def save_sample(self, class_name: str, frames: List[np.ndarray]) -> Path:
        """Guarda una muestra como archivo .npy con timestamp."""
        timestamp = int(time.time())
        filename = f"{class_name}_{timestamp}.npy"
        filepath = self.output_dir / filename
        np.save(filepath, np.array(frames))
        return filepath
    
    def replicate_static_frame(self, frame: np.ndarray, target_length: int) -> List[np.ndarray]:
        """Replica un frame estático para igualar la longitud de muestras dinámicas."""
        return [frame.copy() for _ in range(target_length)]


class DisplayManager:
    """Maneja la interfaz visual con OpenCV."""
    
    WINDOW_NAME = "Capturador de Señas"
    
    @staticmethod
    def draw_landmarks(frame, results, drawing_utils, hands_module):
        """Dibuja landmarks y conexiones sobre el frame."""
        if results.multi_hand_landmarks:
            for hand_landmarks in results.multi_hand_landmarks:
                drawing_utils.draw_landmarks(
                    frame, hand_landmarks, hands_module.HAND_CONNECTIONS
                )
    
    @staticmethod
    def show_waiting_screen(frame, class_name: str, captured: int, total: int):
        """Muestra pantalla de espera antes de iniciar captura."""
        cv2.putText(frame, f"Clase: {class_name}", (10, 30),
                   Config.FONT, Config.FONT_SCALE_INFO,
                   Config.TEXT_COLOR_SUCCESS, Config.TEXT_THICKNESS)
        cv2.putText(frame, f"Muestras: {captured}/{total}", (10, 60),
                   Config.FONT, Config.FONT_SCALE_INFO,
                   Config.TEXT_COLOR_SUCCESS, Config.TEXT_THICKNESS)
        cv2.putText(frame, "Presione ESPACIO para comenzar", (10, 90),
                   Config.FONT, Config.FONT_SCALE_STATUS,
                   Config.TEXT_COLOR_WARNING, Config.TEXT_THICKNESS)
        cv2.putText(frame, "Presione Q para omitir", (10, 120),
                   Config.FONT, Config.FONT_SCALE_STATUS,
                   Config.TEXT_COLOR_ERROR, Config.TEXT_THICKNESS)
    
    @staticmethod
    def show_countdown(frame, value: int):
        """Muestra contador regresivo centrado."""
        position = (frame.shape[1] // 2 - 50, frame.shape[0] // 2)
        cv2.putText(frame, str(value), position,
                   Config.FONT, Config.FONT_SCALE_COUNTDOWN,
                   Config.TEXT_COLOR_COUNTDOWN, Config.TEXT_THICKNESS)
    
    @staticmethod
    def show_capture_progress(frame, class_name: str, current: int, total: int,
                            captured_frames: int, expected_frames: int):
        """Muestra barra de progreso durante la captura."""
        # Información de clase
        cv2.putText(frame, f"Muestra {current}/{total}", (10, 60),
                   Config.FONT, Config.FONT_SCALE_INFO,
                   Config.TEXT_COLOR_SUCCESS, Config.TEXT_THICKNESS)
        
        # Barra de progreso
        progress = min(captured_frames / expected_frames, 1.0) if expected_frames > 0 else 0
        bar_length = 20
        filled = int(progress * bar_length)
        bar = "█" * filled + "░" * (bar_length - filled)
        
        cv2.putText(frame, f"Grabando [{bar}] {captured_frames}/{expected_frames}", (10, 30),
                   Config.FONT, 0.6, Config.TEXT_COLOR_ERROR, 2)
    
    @staticmethod
    def show_inter_sample_message(frame, class_name: str, captured: int, total: int):
        """Muestra mensaje entre muestras."""
        cv2.putText(frame, f"Clase: {class_name} - Progreso: {captured}/{total}", (10, 30),
                   Config.FONT, Config.FONT_SCALE_INFO,
                   Config.TEXT_COLOR_SUCCESS, Config.TEXT_THICKNESS)
        cv2.putText(frame, "Preparando siguiente muestra...", (10, 60),
                   Config.FONT, Config.FONT_SCALE_STATUS,
                   Config.TEXT_COLOR_WARNING, Config.TEXT_THICKNESS)
    
    @staticmethod
    def show_frame(frame):
        """Muestra el frame y retorna la tecla presionada."""
        cv2.imshow(DisplayManager.WINDOW_NAME, frame)
        return cv2.waitKey(1) & 0xFF
    
    @staticmethod
    def cleanup():
        """Libera recursos de OpenCV."""
        cv2.destroyAllWindows()


class MultimodalCapturer:
    """Clase principal para captura automática de muestras multimodales."""
    
    def __init__(self, dataset_path: Path = Config.DEFAULT_DATASET_PATH):
        self.mp_hands = mp.solutions.hands
        self.hands = self.mp_hands.Hands(
            static_image_mode=Config.HANDS_STATIC_MODE,
            max_num_hands=Config.HANDS_MAX_NUM,
            min_detection_confidence=Config.HANDS_MIN_DETECTION_CONFIDENCE,
            min_tracking_confidence=Config.HANDS_MIN_TRACKING_CONFIDENCE
        )
        self.mp_draw = mp.solutions.drawing_utils
        self.capturer = SampleCapturer(dataset_path)
        self.display = DisplayManager()
    
    def capture_samples(
        self,
        class_name: str,
        num_samples: int = Config.DEFAULT_SAMPLES_PER_CLASS,
        duration: float = Config.DEFAULT_DURATION_DYNAMIC,
        fps: int = Config.DEFAULT_FPS,
        is_static: bool = False
    ) -> bool:
        """
        Captura muestras de forma automática para una clase específica.
        
        Args:
            class_name: Nombre de la clase/gesto a capturar.
            num_samples: Número de muestras a capturar.
            duration: Duración en segundos por muestra.
            fps: Frames por segundo para captura dinámica.
            is_static: Indica si el gesto es estático (un solo frame).
        
        Returns:
            True si se completó la captura, False si fue interrumpida.
        """
        self._log_class_header(class_name, num_samples, duration, fps, is_static)
        
        cap = cv2.VideoCapture(0)
        if not cap.isOpened():
            print("Error: No se pudo acceder a la cámara.")
            return False
        
        self._configure_camera(cap)
        
        try:
            return self._capture_loop(
                cap, class_name, num_samples, duration, fps, is_static
            )
        finally:
            cap.release()
            self.display.cleanup()
    
    def _log_class_header(self, class_name: str, num_samples: int, 
                         duration: float, fps: int, is_static: bool):
        """Imprime información de configuración de la clase."""
        gesture_type = "estática" if is_static else "dinámica"
        print(f"\nClase: {class_name.upper()}")
        print(f"Muestras a capturar: {num_samples}")
        print(f"Tipo: {gesture_type} | Duración: {duration}s | FPS: {fps}")
        print("Instrucciones: ESPACIO para iniciar, Q para omitir esta clase.")
    
    def _configure_camera(self, cap):
        """Configura parámetros de la cámara."""
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, Config.FRAME_WIDTH)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, Config.FRAME_HEIGHT)
    
    def _capture_loop(self, cap, class_name: str, num_samples: int,
                     duration: float, fps: int, is_static: bool) -> bool:
        """Bucle principal de captura de muestras."""
        captured = 0
        waiting_start = True
        frames_per_sample = duration * fps if not is_static else 1
        
        while captured < num_samples:
            ret, frame = cap.read()
            if not ret:
                continue
            
            results = self._process_frame(frame)
            self.display.draw_landmarks(frame, results, self.mp_draw, self.mp_hands)
            
            if waiting_start:
                if self._handle_waiting_state(frame, class_name, captured, num_samples):
                    waiting_start = False
                    print(f"Iniciando captura de {num_samples} muestras para '{class_name}'")
                else:
                    continue  # Usuario canceló
            else:
                success = self._capture_single_sample(
                    cap, frame, results, class_name, captured, num_samples,
                    frames_per_sample, fps, is_static
                )
                if success:
                    captured += 1
                else:
                    return False  # Captura interrumpida
                
                # Pausa entre muestras
                if captured < num_samples:
                    self._show_inter_sample_frame(frame, class_name, captured, num_samples)
                    time.sleep(1)
        
        print(f"\nClase '{class_name}' completada: {captured} muestras guardadas.")
        return True
    
    def _process_frame(self, frame):
        """Procesa frame con MediaPipe y retorna resultados."""
        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        return self.hands.process(frame_rgb)
    
    def _handle_waiting_state(self, frame, class_name: str, 
                            captured: int, total: int) -> bool:
        """
        Maneja el estado de espera para iniciar captura.
        
        Returns:
            True si el usuario inició la captura, False si canceló.
        """
        self.display.show_waiting_screen(frame, class_name, captured, total)
        key = self.display.show_frame(frame)
        
        if key == ord(' '):
            return True
        elif key == ord('q'):
            print(f"Clase '{class_name}' omitida por el usuario.")
            return False
        return False
    
    def _capture_single_sample(self, cap, initial_frame, results, class_name: str,
                             sample_index: int, total_samples: int,
                             frames_expected: int, fps: int, is_static: bool) -> bool:
        """
        Captura una sola muestra completa.
        
        Returns:
            True si se guardó exitosamente, False si fue interrumpida.
        """
        print(f"\n  Muestra {sample_index + 1}/{total_samples} - Iniciando en 3", end='')
        
        # Fase de countdown
        if not self._run_countdown(cap, initial_frame, results):
            return False
        
        # Fase de captura
        frames_data = self._capture_frames(cap, results, frames_expected, fps, is_static)
        if frames_data is None:
            return False
        
        # Procesar para gestos estáticos
        if is_static and len(frames_data) == 1:
            frames_data = self.capturer.replicate_static_frame(
                frames_data[0], frames_expected
            )
            print(f"\n    Frame único replicado a {len(frames_data)} frames.")
        
        # Guardar muestra
        filepath = self.capturer.save_sample(class_name, frames_data)
        print(f"    Muestra {sample_index + 1} guardada: {filepath.name} ({len(frames_data)} frames)")
        return True
    
    def _run_countdown(self, cap, frame, results) -> bool:
        """
        Ejecuta countdown de 3 segundos con visualización.
        
        Returns:
            True si completó el countdown, False si fue interrumpido.
        """
        start_time = time.time()
        
        while True:
            elapsed = time.time() - start_time
            remaining = 3 - elapsed
            
            if remaining <= 0:
                print(" ¡Listo!")
                return True
            
            # Actualizar visualización
            ret, frame = cap.read()
            if ret:
                results = self._process_frame(frame)
                self.display.draw_landmarks(frame, results, self.mp_draw, self.mp_hands)
                countdown_value = int(remaining) + 1
                self.display.show_countdown(frame, countdown_value)
                if countdown_value in [2, 1]:
                    print(".", end='', flush=True)
            
            key = self.display.show_frame(frame)
            if key == ord('q'):
                print("\nCaptura interrumpida por el usuario.")
                return False
            
            time.sleep(0.05)  # Pequeña pausa para reducir carga de CPU
    
    def _capture_frames(self, cap, results, frames_expected: int, 
                       fps: int, is_static: bool) -> Optional[List[np.ndarray]]:
        """
        Captura frames durante la duración especificada.
        
        Returns:
            Lista de arrays de landmarks, o None si fue interrumpido.
        """
        frames_data = []
        captured = 0
        start_time = time.time()
        
        if is_static:
            # Para gestos estáticos: capturar un solo frame
            landmarks = self.capturer.landmark_extractor.extract_both_hands(results)
            frames_data.append(landmarks)
            return frames_data
        
        # Para gestos dinámicos: capturar a intervalos regulares
        while captured < frames_expected:
            ret, frame = cap.read()
            if not ret:
                continue
            
            results = self._process_frame(frame)
            self.display.draw_landmarks(frame, results, self.mp_draw, self.mp_hands)
            
            # Verificar si es momento de capturar un frame
            elapsed = time.time() - start_time
            expected_index = int(elapsed * fps)
            
            if expected_index > captured:
                landmarks = self.capturer.landmark_extractor.extract_both_hands(results)
                frames_data.append(landmarks)
                captured += 1
                print(f"    Frame {captured}/{frames_expected}", end='\r')
                
                # Actualizar UI con progreso
                self.display.show_capture_progress(
                    frame, "", 0, 0, captured, frames_expected
                )
            
            # Mostrar frame y verificar interrupción
            key = self.display.show_frame(frame)
            if key == ord('q'):
                print("\nCaptura interrumpida por el usuario.")
                return None
            
            # Control de framerate para evitar consumo excesivo de CPU
            time.sleep(0.01)
        
        return frames_data
    
    def _show_inter_sample_frame(self, frame, class_name: str, 
                               captured: int, total: int):
        """Muestra frame de transición entre muestras."""
        self.display.show_inter_sample_message(frame, class_name, captured, total)
        self.display.show_frame(frame)


def main():
    """Función principal de ejecución."""
    print("CAPTURADOR AUTOMATICO DE SEÑAS MULTIMODAL")
    print("=" * 70)

    # Definir clases a capturar
    classes = [
        # Letras del alfabeto 
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
        "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
        # Palabras comunes (gestos dinámicos)
        "hola", "gracias", "adios", "si", "como_estas"
    ]

    samples_per_class = 100  # Ajustar a 100 para producción
    fps = Config.DEFAULT_FPS
    duration_static = Config.DEFAULT_DURATION_STATIC
    duration_dynamic = Config.DEFAULT_DURATION_DYNAMIC

    print(f"\nConfiguracion:")
    print(f"   Clases a capturar: {len(classes)}")
    print(f"   Muestras por clase: {samples_per_class}")
    print(f"   FPS: {fps}")
    print(f"   Letras: {duration_static}s (gestos estáticos)")
    print(f"   Palabras: {duration_dynamic}s (gestos dinámicos)")
    print("=" * 70)

    capturer = MultimodalCapturer()

    for class_name in classes:
        is_letter = len(class_name) == 1
        duration = duration_static if is_letter else duration_dynamic
        
        print(f"\nPreparando clase: {class_name}")
        input("Presione ENTER para continuar con la captura...")
        
        capturer.capture_samples(
            class_name=class_name,
            num_samples=samples_per_class,
            duration=duration,
            fps=fps,
            is_static=is_letter
        )
        
        print("\n" + "-" * 50)

    print("\nProyecto completado exitosamente.")
    print(f"Archivos guardados en: {capturer.capturer.output_dir}")
    print(f"Total de archivos generados: {len(list(capturer.capturer.output_dir.glob('*.npy')))}")


if __name__ == "__main__":
    main()