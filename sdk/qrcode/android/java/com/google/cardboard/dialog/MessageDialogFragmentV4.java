/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.google.cardboard.dialog;

import android.annotation.SuppressLint;
import android.app.Dialog;
import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import com.google.cardboard.utils.BuildCheck;

public class MessageDialogFragmentV4 extends DialogFragment {
    private static final String TAG = MessageDialogFragmentV4.class.getSimpleName();
    private MessageDialogFragmentV4.MessageDialogListener mDialogListener;

    public static MessageDialogFragmentV4 showDialog(FragmentActivity parent, int requestCode, int id_title, int id_message, String[] permissions) {
        MessageDialogFragmentV4 dialog = newInstance(requestCode, id_title, id_message, permissions);
        dialog.show(parent.getSupportFragmentManager(), TAG);
        return dialog;
    }

    public static MessageDialogFragmentV4 showDialog(Fragment parent, int requestCode, int id_title, int id_message, String[] permissions) {
        MessageDialogFragmentV4 dialog = newInstance(requestCode, id_title, id_message, permissions);
        dialog.setTargetFragment(parent, parent.getId());
        dialog.show(parent.getFragmentManager(), TAG);
        return dialog;
    }

    public static MessageDialogFragmentV4 newInstance(int requestCode, int id_title, int id_message, String[] permissions) {
        MessageDialogFragmentV4 fragment = new MessageDialogFragmentV4();
        Bundle args = new Bundle();
        args.putInt("requestCode", requestCode);
        args.putInt("title", id_title);
        args.putInt("message", id_message);
        args.putStringArray("permissions", permissions != null ? permissions : new String[0]);
        fragment.setArguments(args);
        return fragment;
    }

    public MessageDialogFragmentV4() {
    }

    @SuppressLint({"NewApi"})
    public void onAttach(Context context) {
        super.onAttach(context);
        if (context instanceof MessageDialogFragmentV4.MessageDialogListener) {
            this.mDialogListener = (MessageDialogFragmentV4.MessageDialogListener)context;
        }

        Fragment target;
        if (this.mDialogListener == null) {
            target = this.getTargetFragment();
            if (target instanceof MessageDialogFragmentV4.MessageDialogListener) {
                this.mDialogListener = (MessageDialogFragmentV4.MessageDialogListener)target;
            }
        }

        if (this.mDialogListener == null && BuildCheck.isAndroid4_2()) {
            target = this.getParentFragment();
            if (target instanceof MessageDialogFragmentV4.MessageDialogListener) {
                this.mDialogListener = (MessageDialogFragmentV4.MessageDialogListener)target;
            }
        }

        if (this.mDialogListener == null) {
            throw new ClassCastException(context.toString());
        }
    }

    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Bundle args = savedInstanceState != null ? savedInstanceState : this.getArguments();
        final int requestCode = this.getArguments().getInt("requestCode");
        int id_title = this.getArguments().getInt("title");
        int id_message = this.getArguments().getInt("message");
        final String[] permissions = args.getStringArray("permissions");
        return (new Builder(this.getActivity())).setIcon(17301543).setTitle(id_title).setMessage(id_message).setPositiveButton(17039370, new OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                try {
                    MessageDialogFragmentV4.this.mDialogListener.onMessageDialogResult(MessageDialogFragmentV4.this, requestCode, permissions, true);
                } catch (Exception var4) {
                    Log.w(MessageDialogFragmentV4.TAG, var4);
                }

            }
        }).setNegativeButton(17039360, new OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                try {
                    MessageDialogFragmentV4.this.mDialogListener.onMessageDialogResult(MessageDialogFragmentV4.this, requestCode, permissions, false);
                } catch (Exception var4) {
                    Log.w(MessageDialogFragmentV4.TAG, var4);
                }

            }
        }).create();
    }

    public interface MessageDialogListener {
        void onMessageDialogResult(MessageDialogFragmentV4 var1, int var2, String[] var3, boolean var4);
    }
}

