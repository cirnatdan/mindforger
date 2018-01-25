/*
 note_edit_dialog.cpp     MindForger thinking notebook

 Copyright (C) 2016-2018 Martin Dvorak <martin.dvorak@mindforger.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "note_edit_dialog.h"

using namespace std;

namespace m8r {

NoteEditDialog::NoteEditDialog(Ontology& ontology, QWidget* parent)
    : QDialog{parent}, ontology(ontology)
{
    QGroupBox* basicGroup = new QGroupBox{tr("Basic"), this};

    typeLabel = new QLabel{tr("Type")+":", this};
    typeCombo = new QComboBox{this};
    if(ontology.getNoteTypes().size()) {
        for(const NoteType* t:ontology.getNoteTypes().values()) {
            typeCombo->addItem(QString::fromStdString(t->getName()), QVariant::fromValue<const NoteType*>(t));
        }
        typeCombo->setCurrentText(QString::fromStdString(ontology.getDefaultNoteType()->getName()));
    }

    progressLabel = new QLabel{tr("Progress")+": %", this};
    progressSpin = new QSpinBox{this};
    progressSpin->setMinimum(0);
    progressSpin->setMaximum(100);

    deadlineCheck = new QCheckBox{tr("Deadline")+":", this};
    deadlineEdit = new QDateEdit{QDate::currentDate(), this};

    parentRelLabel = new QLabel{tr("Parent-child Relationship")+":", this};
    parentRelCombo = new QComboBox(this);
    parentRelCombo->addItem(tr("Composition")+" "+QChar(9670));
    parentRelCombo->addItem(tr("Aggregation")+" "+QChar(9671));
    parentRelCombo->addItem(tr("Is-a")+" "+QChar(9651));

    editTagsGroup = new EditTagsPanel{ontology, this};

    QGroupBox* advancedGroup = new QGroupBox{tr("Metadata"), this};

    createdLabel = new QLabel{tr("Created")+":", this};
    createdLine = new QLineEdit{this};
    createdLine->setDisabled(true);
    modifiedPanel = new LabeledEditLinePanel{tr("Last Modified")+":", this};
    modifiedPanel->setEnabled(false);
    readPanel = new LabeledEditLinePanel{tr("Last Read")+":", this};
    readPanel->setEnabled(false);
    readsPanel = new LabeledEditLinePanel{tr("Reads")+":", this};
    readsPanel->setDisabled(true);
    writesPanel = new LabeledEditLinePanel{tr("Writes")+":", this};
    writesPanel->setDisabled(true);
    locationLabel = new QLabel{tr("Location")+":", this};
    locationLine = new QLineEdit{this};
    locationLine->setDisabled(true);

    buttonBox = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel};

    // assembly
    QVBoxLayout* basicLayout = new QVBoxLayout{this};
    basicLayout->addWidget(typeLabel);
    basicLayout->addWidget(typeCombo);
    basicLayout->addWidget(progressLabel);
    basicLayout->addWidget(progressSpin);
    basicLayout->addWidget(deadlineCheck);
    basicLayout->addWidget(deadlineEdit);
    basicLayout->addWidget(parentRelLabel);
    basicLayout->addWidget(parentRelCombo);
    basicGroup->setLayout(basicLayout);
    QVBoxLayout* advancedLayout = new QVBoxLayout{this};
    advancedLayout->addWidget(createdLabel);
    advancedLayout->addWidget(createdLine);
    QWidget* w = new QWidget{this};
    QHBoxLayout* l = new QHBoxLayout(w);
    l->setContentsMargins(0,0,0,0);
    l->addWidget(modifiedPanel);
    l->addWidget(readPanel);
    w->setLayout(l);
    advancedLayout->addWidget(w);
    w = new QWidget{this};
    l = new QHBoxLayout(w);
    l->setContentsMargins(0,0,0,0);
    l->addWidget(readsPanel);
    l->addWidget(writesPanel);
    w->setLayout(l);
    advancedLayout->addWidget(w);
    advancedLayout->addWidget(locationLabel);
    advancedLayout->addWidget(locationLine);
    advancedGroup->setLayout(advancedLayout);
    QVBoxLayout* boxesLayout = new QVBoxLayout{this};
    boxesLayout->addWidget(basicGroup);
    boxesLayout->addWidget(editTagsGroup);
    boxesLayout->addWidget(advancedGroup);
    boxesLayout->addWidget(buttonBox);
    setLayout(boxesLayout);

    // wire signals ensuring that close & set dialog status
    QObject::connect(deadlineCheck, SIGNAL(stateChanged(int)), this, SLOT(handleDeadlineCheck(int)));
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    // IMPROVE extra wiring below to be removed - I was unable to connect QDialog::accept from outside :-/
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &NoteEditDialog::handleAccepted);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &NoteEditDialog::handleRejected);
    //QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, [=](){ qDebug("NoteEditDialog OK"); });

    setWindowTitle(tr("Edit Note Properties"));
    resize(fontMetrics().averageCharWidth()*55, height());
    setModal(true);
}

NoteEditDialog::~NoteEditDialog()
{
    delete typeLabel;
    delete typeCombo;
}

void NoteEditDialog::show()
{
    if(currentNote) {
        // RDWR
        if(currentNote->getType()) {
            int i = typeCombo->findData(QVariant::fromValue<const NoteType*>(currentNote->getType()));
            if(i>=0) {
                typeCombo->setCurrentIndex(i);
            } else {
                qDebug() << "Unknown Note type: " << QString::fromStdString(currentNote->getType()->getName());
            }
        }
        editTagsGroup->refresh(currentNote->getTags());
        progressSpin->setValue(currentNote->getProgress());
        if(currentNote->getDeadline()) {
            deadlineCheck->setChecked(true);
            deadlineEdit->setEnabled(true);

            QDate date{};
            timetToQDate(currentNote->getDeadline(),date);
            deadlineEdit->setDate(date);
        } else {
            deadlineCheck->setChecked(false);
            deadlineEdit->setEnabled(false);

            deadlineEdit->setDate(QDate::currentDate());
        }

        // RDONLY
        createdLine->setText(QString::fromStdString(datetimeToString(currentNote->getCreated())));
        modifiedPanel->setText(QString::fromStdString(datetimeToString(currentNote->getModified())));
        readPanel->setText(QString::fromStdString(datetimeToString(currentNote->getRead())));
        readsPanel->setText(QString::number(currentNote->getReads()));
        writesPanel->setText(QString::number(currentNote->getRevision()));
        locationLine->setText(QString::fromStdString(currentNote->getOutlineKey()));
    }

    QDialog::show();
}

void NoteEditDialog::toNote()
{
    if(currentNote) {
        if(typeCombo->currentIndex() != -1) {
            currentNote->setType((const NoteType*)(typeCombo->itemData(typeCombo->currentIndex(), Qt::UserRole).value<const NoteType*>()));
        }
        currentNote->setTags((editTagsGroup->getTags()));
        currentNote->setProgress(progressSpin->value());
        if(deadlineCheck->isChecked()) {
            tm date {0,0,0,0,0,0,0,0,0,0,0}; // missing initializer required by older GCC versions 4.8.5 and older
            qdateToTm(deadlineEdit->dateTime().date(), date);
            currentNote->setDeadline(datetimeSeconds(&date));
        } else {
            currentNote->setDeadline(0);
        }
    } else {
        qDebug("Attempt to save data from dialog to Note, but no Note is set.");
    }
}

void NoteEditDialog::handleDeadlineCheck(int state)
{
    if(!state) {
        deadlineEdit->setEnabled(false);
    } else {
        deadlineEdit->setEnabled(true);
    }
}

void NoteEditDialog::handleAccepted()
{
    emit acceptedSignal();
}

void NoteEditDialog::handleRejected()
{
    emit rejectedSignal();
}

}
