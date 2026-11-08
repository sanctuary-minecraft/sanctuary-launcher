/* Swing integration check for the adapted window and the upstream GUIHandler. */
import java.awt.Component;
import java.awt.Container;
import java.awt.Graphics2D;
import java.awt.Window;
import java.awt.image.BufferedImage;
import java.io.File;
import java.util.Locale;
import javax.imageio.ImageIO;
import javax.swing.JButton;
import javax.swing.JProgressBar;
import javax.swing.SwingUtilities;
import link.infra.packwiz.installer.ui.data.InstallProgress;
import link.infra.packwiz.installer.ui.gui.GUIHandler;
import link.infra.packwiz.installer.ui.gui.InstallWindow;

public final class InstallWindowCheck {
    private static <T extends Component> T find(Container root, Class<T> type, String text) {
        for (Component child : root.getComponents()) {
            if (type.isInstance(child) && (text == null ||
                    (child instanceof JButton && ((JButton) child).getText().equals(text)))) {
                return type.cast(child);
            }
            if (child instanceof Container) {
                T nested = find((Container) child, type, text);
                if (nested != null) return nested;
            }
        }
        return null;
    }

    private static void require(boolean value, String message) {
        if (!value) throw new AssertionError(message);
    }

    public static void main(String[] args) throws Exception {
        boolean fr = "fr".equals(Locale.getDefault().getLanguage());
        GUIHandler handler = new GUIHandler();
        try {
            handler.setTitle("Sanctuary · packwiz");
            handler.show();
            SwingUtilities.invokeAndWait(() -> {});
            InstallWindow window = null;
            for (Window candidate : Window.getWindows()) {
                if (candidate instanceof InstallWindow) window = (InstallWindow) candidate;
            }
            final InstallWindow frame = window;
            require(frame != null && frame.isVisible(), "Upstream handler did not show branded window");
            handler.submitProgress(new InstallProgress(fr ? "Téléchargement des mods…" : "Downloading mods…", 3, 10));
            SwingUtilities.invokeAndWait(() -> {
                JProgressBar bar = find(frame, JProgressBar.class, null);
                require(bar != null && !bar.isIndeterminate() && bar.getValue() == 3
                        && bar.getMaximum() == 10, "Upstream progress was not preserved");
                frame.showOk(false);
            });
            // Exercise the timer callback from outside the Swing thread.
            frame.timeoutOk(5);
            SwingUtilities.invokeAndWait(() -> {
                JButton next = find(frame, JButton.class, fr ? "Continuer (5)" : "Continue (5)");
                require(next != null, "Countdown or locale broken");
                next.doClick();
                require(handler.getOkButtonPressed(), "Continue no longer reaches upstream handler");
                frame.hideOk();
                // Capture actual Swing layout, including the bundled logo, in both locales.
                BufferedImage image = new BufferedImage(frame.getWidth(), frame.getHeight(), BufferedImage.TYPE_INT_RGB);
                Graphics2D graphics = image.createGraphics();
                frame.paint(graphics);
                graphics.dispose();
                try { ImageIO.write(image, "png", new File(args[0])); }
                catch (Exception error) { throw new RuntimeException(error); }
                frame.showOk(true);
                require(find(frame, JButton.class, fr ? "Annuler" : "Cancel") == null,
                        "Cancel should be hidden during the optional-mod countdown");
                frame.hideOk();
                JButton options = find(frame, JButton.class, fr ? "Mods optionnels…" : "Optional mods…");
                options.doClick();
                require(handler.getOptionsButtonPressed() && !options.isEnabled(), "Options callback broken");
                frame.disableOptionsButton(false);
                require(find(frame, JButton.class, fr ? "Aucun mod optionnel" : "No optional mods") != null,
                        "Empty options label broken");
                find(frame, JButton.class, fr ? "Annuler" : "Cancel").doClick();
                require(handler.getCancelButtonPressed(), "Cancellation callback broken");
                frame.displayProgress(new InstallProgress("Loading…"));
                require(find(frame, JProgressBar.class, null).isIndeterminate(), "Indeterminate progress broken");
            });
            System.out.println("PASS: " + Locale.getDefault() + " logo/progress/options/continue/countdown/cancel");
        } finally {
            handler.dispose();
        }
    }
}
