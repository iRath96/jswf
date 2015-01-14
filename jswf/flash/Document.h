//
//  Document.h
//  jswf/flash
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf_flash__Document__
#define __jswf_flash__Document__

#include <stdio.h>
#include "Reader.h"
#include "Header.h"

#include "Sprite.h"
#include "Tag.h"

#include "Frame.h"

#include <vector>
#include <map>

#include "VM.h"

namespace jswf {
  namespace flash {
    /**
     * Parses and represents a <tt>SWF file</tt>.
     * @todo Document::tags and Sprite::tags ? Somewhat redundant.
     * @see ITagForDocument
     */
    class Document {
    protected:
      Reader reader; //!< The flash::Reader used by this document.
    public:
      Header header; //!< The <tt>HEADER</tt> record for this document.
      avm2::VM avm2;
      
      std::vector<std::shared_ptr<tags::Tag>> tags; //!< A <tt>vector</tt> of <tt>shared_ptr</tt>s to the tags this document contains.
      std::map<uint16_t, std::shared_ptr<Character>> dictionary; //!< The <tt>DICTIONARY</tt> of this document.
      
      Sprite *rootSprite; //!< \todo This is the main_timeline object! It can also be transformed!
      
      Document(std::shared_ptr<jswf::io::GenericReader> reader) : reader(reader) { read(); }
    private:
      void read();
    };
  }
}

#endif