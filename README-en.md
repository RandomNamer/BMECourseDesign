>This file is translated by [auto translation service](https://github.com/RandomNamer/MarkdownTranslator)
<h2 auto_translation: true
align = "center" style="font-family: fantasy;">BMECourseDesign</h2>


### Introduction

The curriculum design of the School of Biomedical Engineering, Shanghai Jiaotong University. The curriculum goals include the use of open source software to build a mini-PACS system and solve practical problems in the practice of biomedical engineering.

### Task 1

Modify and supplement a bitmap processing program using Microsoft Visual C++.

### Task 2

Use DCMTK's practical tools to analyze the DICOM standard using logs and network protocol analyze tools.

### Task 3

Use DCMTK API to write a PACS system.

#### SCU

**Entry file: ** [scu.cpp](3/src/scu.cpp)

**Realized functions:**

- Implement C-STORE request
- Automatically match Presentation Context according to file metadata

#### SCP

**Entry file**:  [scp.cpp](3/src/scp.cpp)

**Realized functions:**

- Implement SCP client that supports C-STORE request
- Realize several kinds of processing to the image, the image is reversed, the image is rotated at any angle, and the image is automatically converted to JPEG compression.
- Implement sqlite3 database records

### Project
- A cross-platform GUI tool for real-time preview and control of compute-intensive image registration process.
- See [registrationViewer](https://github.com/ljjliujunjie123/registrationViewer) for its source.


