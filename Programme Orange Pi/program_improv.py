import numpy as np

def filter_lidar_numpy(points_xy, rayon=150, min_voisins=4):
    """
    Filtre les points isolés d'un LiDAR en utilisant uniquement NumPy.
    Méthode : Radius Outlier Removal (ROR)

    Paramètres:
    - points_xy: Tableau numpy des coordonnées [[x1, y1], [x2, y2], ...]
    - rayon: Le rayon de recherche autour de chaque point.
    - min_voisins: Le nombre minimum de voisins requis dans le rayon.
    """
    # 1. On sépare X et Y pour faciliter le calcul
    x = points_xy[:, 0]
    y = points_xy[:, 1]

    # 2. Calcul matriciel des distances.
    # Cela va créer une grille (Matrice NxN) avec la distance entre chaque point.
    # Pour éviter la racine carrée (gourmande en calcul), on travaille avec les distances au carré (Pythagore : a² + b² = c²)
    dx = x[:, np.newaxis] - x[np.newaxis, :]
    dy = y[:, np.newaxis] - y[np.newaxis, :]

    distance_carree = dx**2 + dy**2
    rayon_carre = rayon**2

    # 3. Pour chaque point, on compte combien de voisins sont à une distance <= au rayon
    # np.sum() sur l'axe 1 compte le nombre de "True" sur chaque ligne de la matrice
    voisins_count = np.sum(distance_carree <= rayon_carre, axis=1)

    # On soustrait 1 car la distance d'un point avec lui-même est 0, il s'est donc compté lui-même
    voisins_count -= 1

    # 4. On crée un masque (liste de True/False) pour les points qui ont assez de voisins
    mask_valides = voisins_count >= min_voisins

    # 5. On sépare les bonnes mesures des fausses
    points_propres = points_xy[mask_valides]
    points_bruit = points_xy[~mask_valides]

    return points_propres, points_bruit

# --- EXEMPLE D'UTILISATION ---

# Conversion basique (Angle + Distance) -> (X, Y)
def polar_to_cartesian(angles_deg, distances):
    angles_rad = np.radians(angles_deg)
    x = distances * np.cos(angles_rad)
    y = distances * np.sin(angles_rad)
    return np.column_stack((x, y))

# (Simulation de données pour l'exemple)
angles_aleatoires = np.random.uniform(0, 360, 500)
distances_aleatoires = np.random.uniform(0, 3000, 500)
points_bruts_xy = polar_to_cartesian(angles_aleatoires, distances_aleatoires)

# Appel de notre fonction 100% numpy
points_filtres, bruit = filter_lidar_numpy(points_bruts_xy, rayon=200, min_voisins=4)

print(f"Total points : {len(points_bruts_xy)}")
print(f"Points gardés : {len(points_filtres)}")
print(f"Points rejetés : {len(bruit)}")