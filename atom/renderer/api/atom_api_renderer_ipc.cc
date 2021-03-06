// Copyright (c) 2013 GitHub, Inc. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/common/api/api_messages.h"
#include "atom/common/native_mate_converters/string16_converter.h"
#include "atom/common/native_mate_converters/value_converter.h"
#include "content/public/renderer/render_view.h"
#include "native_mate/dictionary.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebView.h"

#include "atom/common/node_includes.h"

using content::RenderView;
using WebKit::WebFrame;
using WebKit::WebView;

namespace {

RenderView* GetCurrentRenderView() {
  WebFrame* frame = WebFrame::frameForCurrentContext();
  if (!frame)
    return NULL;

  WebView* view = frame->view();
  if (!view)
    return NULL;  // can happen during closing.

  return RenderView::FromWebView(view);
}

void Send(const string16& channel, const base::ListValue& arguments) {
  RenderView* render_view = GetCurrentRenderView();
  if (render_view == NULL)
    return;

  bool success = render_view->Send(new AtomViewHostMsg_Message(
      render_view->GetRoutingID(), channel, arguments));

  if (!success)
    node::ThrowError("Unable to send AtomViewHostMsg_Message");
}

string16 SendSync(const string16& channel, const base::ListValue& arguments) {
  string16 json;

  RenderView* render_view = GetCurrentRenderView();
  if (render_view == NULL)
    return json;

  IPC::SyncMessage* message = new AtomViewHostMsg_Message_Sync(
      render_view->GetRoutingID(), channel, arguments, &json);
  // Enable the UI thread in browser to receive messages.
  message->EnableMessagePumping();
  bool success = render_view->Send(message);

  if (!success)
    node::ThrowError("Unable to send AtomViewHostMsg_Message_Sync");

  return json;
}

void Initialize(v8::Handle<v8::Object> exports) {
  mate::Dictionary dict(v8::Isolate::GetCurrent(), exports);
  dict.SetMethod("send", &Send);
  dict.SetMethod("sendSync", &SendSync);
}

}  // namespace

NODE_MODULE(atom_renderer_ipc, Initialize)
