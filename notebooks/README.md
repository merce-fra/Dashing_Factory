## Problem Statement

The goal is to develop models capable of discerning normal observations from various anomalies while emphasizing both _**global**_ and _**local explainability**_. It addresses this challenge through multiple perspectives:
1.  **Binary Classification:** Determining the presence or absence of anomalies.
2.  **Multi-class Classification:** Classifying anomalies into :
+ Case 01 : specific categories _(reorder, duplicate, or corrupt)_,
+ Case 02: detecting _high-gravity_ anomalies only.
3.  **Fine-grained Anomaly Detection:** Identifying the exact anomaly type along with the severity level.

### Significance
Timely anomaly detection and root cause analysis in video streaming data contribute significantly to enhancing system reliability and resilience in 5G factory automation environments. Understanding the nature of anomalies facilitates proactive measures, improving operational effectiveness and guiding maintenance schedules.

## Content
The work is organized into notebooks combining code and explanations.

1.  **Preprocessing Notebook:** related to data preprocessing tasks _(c.f. [_preprocessing.ipynb_](_(01)_Preprocessing.ipynb))_.
2.  **EDA Notebook:** conducts exploratory data analysis, providing insights into the dataset's characteristics _(c.f. [_eda.ipynb_](_(02)_eda.ipynb))_.
3.  **FS Notebook:** feature selection tasks _(notebook not yet provided)_.
4.  **Classification Notebooks:** :

    4.1. **Binary Classification Approach :** aiming to differentiate between normal and abnormal samples. Various classification algorithms employed :
    - Gaussian Mixture Models (GMM) &rarr; _c.f. [_(4.1.1)\_gmm\_binary.ipynb_](_(04.1.1)_gmm_binary.ipynb)_
    - Support Vector Machines (SVM) &rarr; _c.f. [_(4.1.2)\_svm\_binary.ipynb_](_(04.1.2)_svm_binary.ipynb)_                                               
    - Neural networks (NN) &rarr; _c.f. [_(4.1.3)\_nn\_binary.ipynb_](_(04.1.3)_nn_binary.ipynb)_
    - TabNet &rarr; _c.f. [_(4.1.4)\_tabnet\_binary.ipynb_](_(04.1.4)_tabnet_binary.ipynb)_
    - XGBoost  &rarr; _c.f. [_(4.1.5)\_xgboost\_binary.ipynb_](_(04.1.5)_xgboost_binary.ipynb)_. 

    4.2. **Multi-class Classification Approach :** extends the anomaly detection framework from simply discerning between normal/anomalous instances &rarr; to categorizing anomalies into distinct classes : _**reorder, duplicate, corrupt**_ anomalies _(or simply _**normal**_ case)_. 

    4.3. **High-Gravity Anomaly Detection Approach :** Identifying anomalies that exhibit particularly _**severe**_ or _**critical characteristics**_, i.e., specifically labeled with a gravity level of +50%. 

    4.4. **Fine-Grained Anomaly Detection Approach :** here we seek to classify anomalies with _**granular detail**_, i.e., considering each anomaly category alongside its associated severity level as a distinct class : Anomaly severity levels ranging from 5% to 50% + Encompassing categories : reorder, duplicate, and corrupt.

  
1. **Discussion and Comparison's notebook :**  Provide a comprehensive evaluation of previous anomaly detection methodologies, comparing their performance across different approaches (binary, multi-class, etc.). Key metrics such as _**accuracy**_ and _**average F-score**_ are utilized to quantify the efficience of each technique &rarr; _c.f. [_(05)\_comparison.ipynb_](_(05)_comparaison.ipynb)_

