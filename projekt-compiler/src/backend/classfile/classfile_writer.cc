/*[--**--]
Copyright (C) 2014  SWPSoSe14Cpp Group

This program is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this
program; if not, see <http://www.gnu.org/licenses/>.*/

/*!
 * \mainpage classfile_writer.cc
 * \author Backend group & friends
 * \date SoSe 2014
 *
 * This class writes the specific class-file we want to create from a rail program.
 * For each function in an rail program (incl. main), we need to produce one class-file.
 *
 */

#include <backend/classfile/classfile_writer.h>
#include <backend/classfile/constant_pool.h>
#include <backend/classfile/Bytecode_writer.h>
#include <array>
#include <iostream>
#include <map>
#include <string>

const char ClassfileWriter::kMagicNumber[] { '\xCA', '\xFE','\xBA', '\xBE' };
const char ClassfileWriter::kNotRequired[] { '\x00', '\x00' };
const char ClassfileWriter::kPublicAccessFlag[] { '\x00', '\x01'};

std::map<ClassfileWriter::ClassfileVersion, std::array<char, 4>>
    ClassfileWriter::kVersionNumbers {
  {ClassfileWriter::ClassfileVersion::JAVA_7,
        std::array<char, 4>{'\x00', '\x00', '\x00', '\x33'}}
};

/*!
 * \brief Constructor for ClassfileWriter
 * \param version The java version of the class-file
 * \param constantPool The specific constant pool for the class-file
 * \param codeFunctions Holds the the bytecode via map function name -> bytecode
 * \param out The ouput stream
 */
ClassfileWriter::ClassfileWriter(ClassfileVersion version,
                                 ConstantPool* constantPool,
                                 Graphs& graphs,
                                 const std::map<std::string, std::vector<char>&> codeFunctions,
                                 std::ostream* out) :
    version_(version), code_functions_(codeFunctions), out_(out), writer(out) {
  constant_pool_ = std::make_shared<ConstantPool>(*constantPool);
  graphs_ = graphs;
}

/*!
 * \brief Deconstructor for ClassFileWriter
 */
ClassfileWriter::~ClassfileWriter() {
}

/*!
 * \brief Calls methods to write into the classfile
 * Each method represents an specific part of the class-file
 */
void ClassfileWriter::WriteClassfile() {
 //WriteMagicNumber();
  //WriteVersionNumber();
  WriteConstantPool();
  //WriteAccessFlags();
  //WriteClassName();
  //WriteSuperClassName();
  //WriteInterfaces();
  //WriteFields();
  //WriteMethods();
}

/*!
 * \brief Write the magic number
 * The magic number indicates a java class-file
 */
void ClassfileWriter::WriteMagicNumber() {
  out_->write(kMagicNumber, (sizeof(kMagicNumber)/sizeof(kMagicNumber[0])));
}

/*!
 * \brief Write the java version number (e.g. 0x00000033 for v.7)
 */
void ClassfileWriter::WriteVersionNumber() {
  out_->write(kVersionNumbers[version_].data(),
              kVersionNumbers[version_].size());
}

/*!
 * \brief Write the constant pool
 * \sa constant_pool.cc
 */
void ClassfileWriter::WriteConstantPool() {
  size_t size = constant_pool_->size() + 1;
  writer.writeU16(size);
  writer.writeVector(constant_pool_->getByteArray());
}

/*!
 * \brief Write the access flag (e.g. 0x00000001 for public)
 */
void ClassfileWriter::WriteAccessFlags() {
  out_->write(kPublicAccessFlag,
              (sizeof(kPublicAccessFlag)/sizeof(kPublicAccessFlag[0])));
}

/*!
 * \brief Write the class name
 * we call our outfile Main.class. therefore every classname is Main
 */
void ClassfileWriter::WriteClassName() {
	writer.writeU16(constant_pool_->addClassRef(constant_pool_->addString("Main")));
}
/*!
 * \brief Write super class name
 * For us we always have the java/lang/Object class
 */
void ClassfileWriter::WriteSuperClassName() {
	writer.writeU16(constant_pool_->addClassRef(constant_pool_->addString("java/lang/Object")));
}

/*!
 * \brief Write the interfaces
 * Not used in Rail programms, thus 0x0000
 */
void ClassfileWriter::WriteInterfaces() {
  out_->write(kNotRequired, (sizeof(kNotRequired) / sizeof(kNotRequired[0])));
}

/*!
 * \brief Write the fields
 * Not used in Rail programms, thus 0x0000
 */
void ClassfileWriter::WriteFields() {
  out_->write(kNotRequired, (sizeof(kNotRequired) / sizeof(kNotRequired[0])));
}

/*!
 * \brief Write the methods
 * The methods refer to the constant pool.
 * BUT this reference is again an reference to the actual element.
 * (i.e. method -> reference in constant pool -> actual element in constant pool)
 * FIXME: The init and main is hard coded. Should be replaced later.
 */
void ClassfileWriter::WriteMethods() {
  std::vector<std::string> keys = this->graphs_.keyset();
  // plus 1 for the init method
  size_t size = keys.size();
  writer.writeU16(size+1);
  WriteInitMethod();
  for(std::vector<std::string>::size_type i = 0; i != keys.size(); i++) {
    *out_<< kPublicAccessFlag;
    writer.writeU16(constant_pool_->addString(keys[i]));
    writer.writeU16(constant_pool_->addString("()V"));
    WriteAttributes();
  }

  // std::vector<char> func = code_functions_.at("main");
  // out_.write((char*)func.data(),
  //          func.size());
}
/*!
 * \brief Writes the <init> in class-file
 * Is the same in all java classes we generate
 */
void ClassfileWriter::WriteInitMethod(){
	out_->write(kPublicAccessFlag, (sizeof(kPublicAccessFlag)/sizeof(kPublicAccessFlag[0])));
	writer.writeU16(constant_pool_->addString("<init>"));
	writer.writeU16(constant_pool_->addString("()V"));
	/* WriteAttributes */
	char initCode[]{'\x2a','\xb7','\x00','\x01','\xb1'};
	int16_t initCodeCount = sizeof(initCode)/sizeof(initCode[0]);
	// attribute_count=1
	writer.writeU16(1);
	writer.writeU16(constant_pool_->addString("Code"));
	writer.writeU32(17);
	// max_stack=1
	writer.writeU16(1);
	// max_locals=1
	writer.writeU16(1);
	// code_length=5
	writer.writeU32(initCodeCount);
	out_->write(initCode, (initCodeCount));
	// exception_table_length=0
	*out_<< kNotRequired;
	// attributes_count
	*out_<< kNotRequired;
	}

/*!
 * \brief Writes attributes in class-file
 * Every method calls WritesAttributes
 */
void ClassfileWriter::WriteAttributes() {
}
