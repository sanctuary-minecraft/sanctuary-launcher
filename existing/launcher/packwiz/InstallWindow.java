/*
 * Adapted from packwiz-installer v0.5.14 InstallWindow.kt.
 * Copyright (c) 2021 comp500. MIT license, see LICENSE.
 * Sanctuary branding: KOKA99CAB, 2026.
 */
package link.infra.packwiz.installer.ui.gui;

import link.infra.packwiz.installer.ui.data.InstallProgress;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Image;
import java.util.Locale;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.SwingUtilities;
import javax.swing.border.EmptyBorder;

/** Presentation only: GUIHandler retains all packwiz installation decisions. */
public final class InstallWindow extends JFrame {
    private final boolean french = "fr".equals(Locale.getDefault().getLanguage());
    private final JLabel progressLabel;
    private final JProgressBar progressBar;
    private final JButton options;
    private final JButton cancel;
    private final JButton ok;
    private final JPanel buttons;

    public InstallWindow(GUIHandler handler) {
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setResizable(false);

        ImageIcon logo = new ImageIcon(InstallWindow.class.getResource("/sanctuary/packwiz/logo.png"));
        JPanel header = new JPanel(new BorderLayout());
        header.setBackground(Color.BLACK);
        header.setBorder(new EmptyBorder(14, 24, 4, 24));
        JLabel wordmark = new JLabel(new ImageIcon(
                logo.getImage().getScaledInstance(640, 160, Image.SCALE_AREA_AVERAGING)));
        wordmark.getAccessibleContext().setAccessibleName("Sanctuary");
        header.add(wordmark, BorderLayout.CENTER);
        add(header, BorderLayout.NORTH);

        JPanel progress = new JPanel(new BorderLayout(0, 10));
        progress.setBorder(new EmptyBorder(20, 24, 12, 24));
        progressBar = new JProgressBar();
        progressBar.setIndeterminate(true);
        progressBar.getAccessibleContext().setAccessibleName(text("Progression", "Progress"));
        progressLabel = new JLabel(text("Chargement du pack…", "Loading the pack…"));
        // Keep long filenames from widening the window beyond the screen.
        progressLabel.setPreferredSize(new Dimension(640, 24));
        progress.add(progressBar, BorderLayout.NORTH);
        progress.add(progressLabel, BorderLayout.SOUTH);
        add(progress, BorderLayout.CENTER);

        buttons = new JPanel(new FlowLayout(FlowLayout.RIGHT, 8, 0));
        buttons.setBorder(new EmptyBorder(0, 16, 18, 16));
        options = new JButton(text("Mods optionnels…", "Optional mods…"));
        options.addActionListener(event -> {
            options.setText(text("Chargement…", "Loading…"));
            options.setEnabled(false);
            handler.setOptionsButtonPressed(true);
        });
        cancel = new JButton(text("Annuler", "Cancel"));
        cancel.addActionListener(event -> {
            cancel.setEnabled(false);
            handler.setCancelButtonPressed(true);
        });
        ok = new JButton(text("Continuer", "Continue"));
        ok.addActionListener(event -> handler.setOkButtonPressed(true));
        buttons.add(options);
        buttons.add(cancel);
        add(buttons, BorderLayout.SOUTH);
        pack();
        setLocationRelativeTo(null);
    }

    private String text(String fr, String en) {
        return french ? fr : en;
    }

    public final void displayProgress(InstallProgress progress) {
        progressBar.setIndeterminate(!progress.getHasProgress());
        progressBar.setStringPainted(progress.getHasProgress());
        if (progress.getHasProgress()) {
            progressBar.setMaximum(progress.getProgressTotal());
            progressBar.setValue(progress.getProgress());
            progressBar.setString(progress.getProgress() + " / " + progress.getProgressTotal());
        } else {
            progressBar.setValue(0);
        }
        progressLabel.setText(progress.getMessage());
        progressLabel.setToolTipText(progress.getMessage());
    }

    public final void disableOptionsButton(boolean hasOptions) {
        options.setText(hasOptions ? text("Mods optionnels…", "Optional mods…")
                : text("Aucun mod optionnel", "No optional mods"));
        options.setEnabled(false);
    }

    public final void showOk(boolean hideCancel) {
        if (hideCancel) {
            buttons.remove(cancel);
        }
        if (ok.getParent() != buttons) {
            buttons.add(ok);
        }
        buttons.revalidate();
        buttons.repaint();
    }

    public final void hideOk() {
        buttons.remove(ok);
        if (cancel.getParent() != buttons) {
            buttons.add(cancel);
        }
        buttons.revalidate();
        buttons.repaint();
    }

    public final void timeoutOk(long remaining) {
        // Upstream GUIHandler invokes this callback from its countdown timer.
        if (!SwingUtilities.isEventDispatchThread()) {
            SwingUtilities.invokeLater(() -> timeoutOk(remaining));
            return;
        }
        ok.setText(text("Continuer", "Continue") + " (" + remaining + ")");
    }
}
