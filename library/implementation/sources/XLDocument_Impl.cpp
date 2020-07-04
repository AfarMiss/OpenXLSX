//
// Created by Troldal on 24/07/16.
//

#include "XLContentTypes_Impl.hpp"
#include "XLDocument_Impl.hpp"
#include "XLWorksheet_Impl.hpp"
#include "XLTemplate_Impl.hpp"

//#include <pugixml.hpp>
#include <XlQuery_Impl.hpp>

using namespace std;
using namespace OpenXLSX;


/**
 * @details The default constructor, with no arguments.
 */
Impl::XLDocument::XLDocument()
        : m_filePath(""),
          m_documentRelationships(nullptr),
          m_contentTypes(nullptr),
          m_docAppProperties(nullptr),
          m_docCoreProperties(nullptr),
          m_workbook(nullptr){
}

/**
 * @details An alternative constructor, taking a std::string with the path to the .xlsx package as an argument.
 */
Impl::XLDocument::XLDocument(const std::string& docPath)
        : m_filePath(""),
          m_documentRelationships(nullptr),
          m_contentTypes(nullptr),
          m_docAppProperties(nullptr),
          m_docCoreProperties(nullptr),
          m_workbook(nullptr) {

    OpenDocument(docPath);
}

/**
 * @details The destructor calls the closeDocument method before the object is destroyed.
 */
Impl::XLDocument::~XLDocument() {

    CloseDocument();
}

/**
 * @details The openDocument method opens the .xlsx package in the following manner:
 * - Check if a document is already open. If yes, close it.
 * - Create a temporary folder for the contents of the .xlsx package
 * - Unzip the contents of the package to the temporary folder.
 * - load the contents into the datastructure for manipulation.
 */
void Impl::XLDocument::OpenDocument(const string& fileName) {
    // Check if a document is already open. If yes, close it.
    // TODO: Consider throwing if a file is already open.
    if (m_archive.isOpen())
        CloseDocument();

    m_filePath = fileName;
    m_archive.openArchive(m_filePath);


    // ===== Open the Relationships and Content_Types files for the document level.
    m_documentRelationships =
            std::make_unique<XLRelationships>(*this, "_rels/.rels");

    m_contentTypes =
            std::make_unique<XLContentTypes>(*this, "[Content_Types].xml");

    // ===== Open the workbook and document property items
    m_docCoreProperties =
            std::make_unique<XLCoreProperties>(*this,m_documentRelationships->RelationshipByTarget("docProps/core.xml").Target().value());

    m_docAppProperties =
            std::make_unique<XLAppProperties>(*this,m_documentRelationships->RelationshipByTarget("docProps/app.xml").Target().value());

    m_workbook =
            std::make_unique<XLWorkbook>(*this,m_documentRelationships->RelationshipByTarget("xl/workbook.xml").Target().value());

}

/**
 * @details Create a new document. This is done by saving the data in XLTemplate.h in binary format.
 */
void Impl::XLDocument::CreateDocument(const std::string& fileName) {

    std::ofstream outfile(fileName, std::ios::binary);
    // TODO: Resource leak in case of an exception.

    // ===== Casting, in particular reinterpret_cast, is discouraged, but in this case it is unfortunately unavoidable.
    outfile.write(reinterpret_cast<const char*>(excelTemplate.data()), excelTemplate.size());
    outfile.close();

    OpenDocument(fileName);
}

/**
 * @details The document is closed by deleting the temporary folder structure.
 * @todo Consider deleting all the internal objects as well.
 */
void Impl::XLDocument::CloseDocument() {

    m_archive.closeArchive();
    m_filePath.clear();
    m_documentRelationships.reset(nullptr);
    m_contentTypes.reset(nullptr);
    m_docAppProperties.reset(nullptr);
    m_docCoreProperties.reset(nullptr);
    m_workbook.reset(nullptr);
}

/**
 * @details Save the document with the same name. The existing file will be overwritten.
 */
bool Impl::XLDocument::SaveDocument() {

    return SaveDocumentAs(m_filePath);
}

/**
 * @details Save the document with a new name. If present, the 'calcChain.xml file will be ignored. The reason for this
 * is that changes to the document may invalidate the calcChain.xml file. Deleting will force Excel to re-create the
 * file. This will happen automatically, without the user noticing.
 */
bool Impl::XLDocument::SaveDocumentAs(const string& fileName) {

    m_filePath = fileName;

    // Commit all XML files, i.e. save to the zip file.
    m_documentRelationships->WriteXMLData();
    m_contentTypes->WriteXMLData();
    m_docAppProperties->WriteXMLData();
    m_docCoreProperties->WriteXMLData();
    m_workbook->WriteXMLData();

    m_archive.saveArchive(m_filePath);

    return true;
}

/**
 * @details
 * @todo Currently, this method returns the full path, which is not the intention.
 */
const string& Impl::XLDocument::DocumentName() const {

    return m_filePath;
}

/**
 * @details
 */
const string& Impl::XLDocument::DocumentPath() const {

    return m_filePath;
}

/**
 * @details Get a pointer to the underlying XLWorkbook object.
 */
Impl::XLWorkbook* Impl::XLDocument::Workbook() {

    return m_workbook.get();
}

/**
 * @details Get a const pointer to the underlying XLWorkbook object.
 */
const Impl::XLWorkbook* Impl::XLDocument::Workbook() const {

    return m_workbook.get();
}

/**
 * @details Get the value for a property.
 */
std::string Impl::XLDocument::GetProperty(XLProperty theProperty) const {

    switch (theProperty) {
        case XLProperty::Application :
            return m_docAppProperties->Property("Application").text().get();

        case XLProperty::AppVersion :
            return m_docAppProperties->Property("AppVersion").text().get();

        case XLProperty::Category :
            return m_docCoreProperties->Property("cp:category").text().get();

        case XLProperty::Company :
            return m_docAppProperties->Property("Company").text().get();

        case XLProperty::CreationDate :
            return m_docCoreProperties->Property("dcterms:created").text().get();

        case XLProperty::Creator :
            return m_docCoreProperties->Property("dc:creator").text().get();

        case XLProperty::Description :
            return m_docCoreProperties->Property("dc:description").text().get();

        case XLProperty::DocSecurity :
            return m_docAppProperties->Property("DocSecurity").text().get();

        case XLProperty::HyperlinkBase :
            return m_docAppProperties->Property("HyperlinkBase").text().get();

        case XLProperty::HyperlinksChanged :
            return m_docAppProperties->Property("HyperlinksChanged").text().get();

        case XLProperty::Keywords :
            return m_docCoreProperties->Property("cp:keywords").text().get();

        case XLProperty::LastModifiedBy :
            return m_docCoreProperties->Property("cp:lastModifiedBy").text().get();

        case XLProperty::LastPrinted :
            return m_docCoreProperties->Property("cp:lastPrinted").text().get();

        case XLProperty::LinksUpToDate :
            return m_docAppProperties->Property("LinksUpToDate").text().get();

        case XLProperty::Manager :
            return m_docAppProperties->Property("Manager").text().get();

        case XLProperty::ModificationDate :
            return m_docCoreProperties->Property("dcterms:modified").text().get();

        case XLProperty::ScaleCrop :
            return m_docAppProperties->Property("ScaleCrop").text().get();

        case XLProperty::SharedDoc :
            return m_docAppProperties->Property("SharedDoc").text().get();

        case XLProperty::Subject :
            return m_docCoreProperties->Property("dc:subject").text().get();

        case XLProperty::Title :
            return m_docCoreProperties->Property("dc:title").text().get();

    }

    return ""; // To silence compiler warning.
}

/**
 * @details Set the value for a property.
 *
 * If the property is a datetime, it must be in the W3CDTF format, i.e. YYYY-MM-DDTHH:MM:SSZ. Also, the time should
 * be GMT. Creating a time point in this format can be done as follows:
 * ```cpp
 * #include <iostream>
 * #include <iomanip>
 * #include <ctime>
 * #include <sstream>
 *
 * // other stuff here
 *
 * std::time_t t = std::time(nullptr);
 * std::tm tm = *std::gmtime(&t);
 *
 * std::stringstream ss;
 * ss << std::put_time(&tm, "%FT%TZ");
 * auto datetime = ss.str();
 *
 * // datetime now is a string with the datetime in the right format.
 *
 * ```
 */
void Impl::XLDocument::SetProperty(XLProperty theProperty, const string& value) {

    switch (theProperty) {
        case XLProperty::Application :
            m_docAppProperties->SetProperty("Application", value);
            break;

        case XLProperty::AppVersion : // ===== TODO: Clean up this section
            try {
                std::stof(value);
            }
            catch (...) {
                throw XLException("Invalid property value");
            }

            if (value.find('.') != std::string::npos) {
                if (!value.substr(value.find('.') + 1).empty() && value.substr(value.find('.') + 1).size() <= 5) {
                    if (!value.substr(0, value.find('.')).empty() && value.substr(0, value.find('.')).size() <= 2) {

                        m_docAppProperties->SetProperty("AppVersion", value);
                    }
                    else
                        throw XLException("Invalid property value");
                }
                else
                    throw XLException("Invalid property value");
            }
            else
                throw XLException("Invalid property value");

            break;

        case XLProperty::Category :
            m_docCoreProperties->SetProperty("cp:category", value);
            break;

        case XLProperty::Company :
            m_docAppProperties->SetProperty("Company", value);
            break;

        case XLProperty::CreationDate :
            m_docCoreProperties->SetProperty("dcterms:created", value);
            break;

        case XLProperty::Creator :
            m_docCoreProperties->SetProperty("dc:creator", value);
            break;

        case XLProperty::Description :
            m_docCoreProperties->SetProperty("dc:description", value);
            break;

        case XLProperty::DocSecurity :
            if (value == "0" || value == "1" || value == "2" || value == "4" || value == "8")
                m_docAppProperties->SetProperty("DocSecurity", value);
            else
                throw XLException("Invalid property value");
            break;

        case XLProperty::HyperlinkBase :
            m_docAppProperties->SetProperty("HyperlinkBase", value);
            break;

        case XLProperty::HyperlinksChanged :
            if (value == "true" || value == "false")
                m_docAppProperties->SetProperty("HyperlinksChanged", value);
            else
                throw XLException("Invalid property value");

            break;

        case XLProperty::Keywords :
            m_docCoreProperties->SetProperty("cp:keywords", value);
            break;

        case XLProperty::LastModifiedBy :
            m_docCoreProperties->SetProperty("cp:lastModifiedBy", value);
            break;

        case XLProperty::LastPrinted :
            m_docCoreProperties->SetProperty("cp:lastPrinted", value);
            break;

        case XLProperty::LinksUpToDate :
            if (value == "true" || value == "false")
                m_docAppProperties->SetProperty("LinksUpToDate", value);
            else
                throw XLException("Invalid property value");
            break;

        case XLProperty::Manager :
            m_docAppProperties->SetProperty("Manager", value);
            break;

        case XLProperty::ModificationDate :
            m_docCoreProperties->SetProperty("dcterms:modified", value);
            break;

        case XLProperty::ScaleCrop :
            if (value == "true" || value == "false")
                m_docAppProperties->SetProperty("ScaleCrop", value);
            else
                throw XLException("Invalid property value");
            break;

        case XLProperty::SharedDoc :
            if (value == "true" || value == "false")
                m_docAppProperties->SetProperty("SharedDoc", value);
            else
                throw XLException("Invalid property value");
            break;

        case XLProperty::Subject :
            m_docCoreProperties->SetProperty("dc:subject", value);
            break;

        case XLProperty::Title :
            m_docCoreProperties->SetProperty("dc:title", value);
            break;
    }
}

/**
 * @details Delete a property
 */
void Impl::XLDocument::DeleteProperty(XLProperty theProperty) {

    SetProperty(theProperty, "");
}

/**
 * @brief
 * @param command
 */
void Impl::XLDocument::executeCommand(XLCommand command) {

    switch(command.commandType()) {

        case XLCommandType::SetSheetColor :
            break;

        case XLCommandType::SetSheetName :
            setSheetName(command);
            break;

        case XLCommandType::DeleteSheet :
            break;

        case XLCommandType::CloneSheet :
            break;

        case XLCommandType::SetSheetVisibility :
            m_workbook->executeCommand(command);
            break;

        default:
            break;

    }
}

std::string Impl::XLDocument::executeQuery(XLQuery query) const {

    switch (query.queryType()) {

        case XLQueryType::GetSheetName:
            return m_workbook->queryCommand(query);

        case XLQueryType::GetSheetIndex:
            return m_workbook->queryCommand(query);

        case XLQueryType::GetSheetVisibility:
            return m_workbook->queryCommand(query);

        default:
            return std::string();

    }


}

/**
 * @details Get a pointer to the sheet node in the app.xml file.
 */
XMLNode Impl::XLDocument::SheetNameNode(const std::string& sheetName) {

    return m_docAppProperties->SheetNameNode(sheetName);
}

/**
 * @details Get a pointer to the content item in the [Content_Types].xml file.
 */
Impl::XLContentItem Impl::XLDocument::ContentItem(const std::string& path) {

    return m_contentTypes->ContentItem(path);
}

/**
 * @details Ad a new ContentItem and return the resulting object.
 */
Impl::XLContentItem Impl::XLDocument::AddContentItem(const std::string& contentPath, XLContentType contentType) {

    m_contentTypes->AddOverride(contentPath, contentType);
    return m_contentTypes->ContentItem(contentPath);
}

void Impl::XLDocument::DeleteContentItem(Impl::XLContentItem& item) {

    m_contentTypes->DeleteOverride(item);
}

/**
 * @details Add a xml file to the package.
 * @warning The content input parameter must remain valud
 */
void Impl::XLDocument::AddOrReplaceXMLFile(const std::string& path, const std::string& content) {

    m_archive.addEntry(path, content);
}

/**
 * @details
 */
std::string Impl::XLDocument::GetXMLFile(const std::string& path) {

    return (m_archive.hasEntry(path) ? m_archive.getEntry(path) : "");
}

/**
 * @details
 */
void Impl::XLDocument::DeleteXMLFile(const std::string& path) {

    m_archive.deleteEntry(path);
}

/**
 * @details Get a pointer to the XLAppProperties object
 */
Impl::XLAppProperties* Impl::XLDocument::AppProperties() {

    return m_docAppProperties.get();
}

/**
 * @details Get a pointer to the (const) XLAppProperties object
 */
const Impl::XLAppProperties* Impl::XLDocument::AppProperties() const {

    return m_docAppProperties.get();
}

/**
 * @details Get a pointer to the XLCoreProperties object
 */
Impl::XLCoreProperties* Impl::XLDocument::CoreProperties() {

    return m_docCoreProperties.get();
}

/**
 * @details Get a pointer to the (const) XLCoreProperties object
 */
const Impl::XLCoreProperties* Impl::XLDocument::CoreProperties() const {

    return m_docCoreProperties.get();
}

void Impl::XLDocument::setSheetName(XLCommand command) {
    m_docAppProperties->SetSheetName(m_workbook->getSheetName(command.sender()), command.parameter());
    m_workbook->executeCommand(command);
}
